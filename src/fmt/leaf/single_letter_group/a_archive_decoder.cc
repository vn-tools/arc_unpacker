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

bool AArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    AArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_u16_le();
    const auto offset_to_data = input_file.stream.tell() + 32 * file_count;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = input_file.stream.read_to_zero(23).str();
        const auto tmp = input_file.stream.read_u8();
        entry->compressed = tmp > 0;
        entry->size = input_file.stream.read_u32_le();
        entry->offset = input_file.stream.read_u32_le() + offset_to_data;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> AArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);

    bstr data;
    if (entry->compressed)
    {
        const auto size_orig = input_file.stream.read_u32_le();
        data = input_file.stream.read(entry->size-4);
        data = util::pack::lzss_decompress_bytewise(data, size_orig);
    }
    else
        data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> AArchiveDecoder::get_linked_formats() const
{
    return {"leaf/w"};
}

static auto dummy = fmt::register_fmt<AArchiveDecoder>("leaf/a");
