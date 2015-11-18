#include "fmt/leaf/single_letter_group/a_archive_decoder.h"
#include "util/pack/lzss.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "\x1E\xAF"_b; // LEAF in hexspeak

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        bool compressed;
    };
}

bool AArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    AArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    const auto file_count = arc_file.io.read_u16_le();
    const auto offset_to_data = arc_file.io.tell() + 32 * file_count;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.io.read_to_zero(23).str();
        const auto tmp = arc_file.io.read_u8();
        entry->compressed = tmp > 0;
        entry->size = arc_file.io.read_u32_le();
        entry->offset = arc_file.io.read_u32_le() + offset_to_data;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> AArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);

    bstr data;
    if (entry->compressed)
    {
        const auto size_orig = arc_file.io.read_u32_le();
        data = arc_file.io.read(entry->size-4);
        data = util::pack::lzss_decompress_bytewise(data, size_orig);
    }
    else
        data = arc_file.io.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<AArchiveDecoder>("leaf/a");
