#include "fmt/leaf/kcap_archive_decoder.h"
#include "err.h"
#include "log.h"
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
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        bool compressed;
    };
}

static size_t detect_version(File &arc_file, const size_t file_count)
{
    size_t version = 0;
    arc_file.io.peek(arc_file.io.tell(), [&]()
    {
        arc_file.io.skip((file_count - 1) * (24 + 8));
        arc_file.io.skip(24);
        const auto last_entry_offset = arc_file.io.read_u32_le();
        const auto last_entry_size = arc_file.io.read_u32_le();
        if (last_entry_offset + last_entry_size == arc_file.io.size())
            version = 1;
    });
    arc_file.io.peek(arc_file.io.tell(), [&]()
    {
        arc_file.io.skip((file_count - 1) * (4 + 24 + 8));
        arc_file.io.skip(4 + 24);
        const auto last_entry_offset = arc_file.io.read_u32_le();
        const auto last_entry_size = arc_file.io.read_u32_le();
        if (last_entry_offset + last_entry_size == arc_file.io.size())
            version = 2;
    });
    return version;
}

static std::unique_ptr<fmt::ArchiveMeta> read_meta_v1(
    File &arc_file, const size_t file_count)
{
    auto meta = std::make_unique<fmt::ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->compressed = true;
        entry->name = util::sjis_to_utf8(arc_file.io.read_to_zero(24)).str();
        entry->offset = arc_file.io.read_u32_le();
        entry->size = arc_file.io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

static std::unique_ptr<fmt::ArchiveMeta> read_meta_v2(
    File &arc_file, const size_t file_count)
{
    auto meta = std::make_unique<fmt::ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        const auto type = static_cast<EntryType>(arc_file.io.read_u32_le());
        entry->name = util::sjis_to_utf8(arc_file.io.read_to_zero(24)).str();
        entry->offset = arc_file.io.read_u32_le();
        entry->size = arc_file.io.read_u32_le();
        if (type == EntryType::RegularFile)
            entry->compressed = false;
        else if (type == EntryType::CompressedFile)
            entry->compressed = true;
        else
        {
            if (!entry->size)
                continue;
            Log.warn("Unknown entry type: %08x\n", type);
        }
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

bool KcapArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    KcapArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    const auto file_count = arc_file.io.read_u32_le();
    const auto version = detect_version(arc_file, file_count);
    if (version == 1)
        return read_meta_v1(arc_file, file_count);
    else if (version == 2)
        return read_meta_v2(arc_file, file_count);
    else
        throw err::UnsupportedVersionError(version);
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
    return { "truevision/tga", "leaf/bbm", "leaf/bjr" };
}

static auto dummy = fmt::register_fmt<KcapArchiveDecoder>("leaf/kcap");
