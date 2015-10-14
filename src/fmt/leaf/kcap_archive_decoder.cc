#include "fmt/leaf/kcap_archive_decoder.h"
#include "err.h"
#include "fmt/truevision/tga_image_decoder.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/format.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "KCAP"_b;

namespace
{
    enum class EntryType : u32
    {
        RegularFile    = 0x00000000,
        CompressedFile = 0x00000001,
        Folder         = 0xCCCCCCCC,
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        bool compressed;
    };
}

bool KcapArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    KcapArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    auto file_count = arc_file.io.read_u32_le();

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        const auto type = static_cast<EntryType>(arc_file.io.read_u32_le());
        entry->name = util::sjis_to_utf8(arc_file.io.read_to_zero(24)).str();
        entry->offset = arc_file.io.read_u32_le();
        entry->size = arc_file.io.read_u32_le();

        if (type == EntryType::Folder)
            continue;
        else if (type == EntryType::RegularFile)
            entry->compressed = false;
        else if (type == EntryType::CompressedFile)
            entry->compressed = true;
        else
        {
            throw err::CorruptDataError(
                util::format("Unknown entry type: %08x", type));
        }

        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> KcapArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    bstr data;
    if (entry->compressed)
    {
        auto size_comp = arc_file.io.read_u32_le();
        auto size_orig = arc_file.io.read_u32_le();
        data = arc_file.io.read(size_comp - 8);
        data = util::pack::lzss_decompress_bytewise(data, size_orig);
    }
    else
        data = arc_file.io.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> KcapArchiveDecoder::get_linked_formats() const
{
    return { "truevision/tga" };
}

static auto dummy = fmt::register_fmt<KcapArchiveDecoder>("leaf/kcap");
