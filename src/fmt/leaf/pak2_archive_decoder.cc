#include "fmt/leaf/pak2_archive_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool Pak2ArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    if (!arc_file.has_extension("pak"))
        return false;
    const auto file_count = arc_file.io.seek(0x1C).read_u16_le();
    if (file_count < 2)
        return false;
    const auto value1 = arc_file.io.seek(0x34).read_u32_le();
    const auto value2 = arc_file.io.seek(0x3C).read_u32_le();
    for (const auto i : util::range(file_count))
    {
        if (arc_file.io.seek(0x34 + i * 0x20).read_u32_le() != value1)
            return false;
        if (arc_file.io.seek(0x3C + i * 0x20).read_u32_le() != value2)
            return false;
    }
    return true;
}

std::unique_ptr<fmt::ArchiveMeta>
    Pak2ArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    const auto file_count = arc_file.io.seek(0x1C).read_u16_le();
    const auto data_offset = 0x20 + 0x20 * file_count;
    arc_file.io.seek(0x20);
    for (const auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        arc_file.io.skip(2);
        auto name = arc_file.io.read(12);
        for (auto &c : name)
            c = (c << 4) | (c >> 4);
        entry->name = name.str(true);
        if (entry->name.empty())
            continue;
        arc_file.io.skip(2);
        entry->offset = arc_file.io.read_u32_le() + data_offset;
        arc_file.io.skip(4);
        entry->size = arc_file.io.read_u32_le();
        arc_file.io.skip(4);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> Pak2ArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = arc_file.io.seek(entry->offset).read(entry->size);
    return std::make_unique<File>(e.name, data);
}

std::vector<std::string> Pak2ArchiveDecoder::get_linked_formats() const
{
    return
    {
        "leaf/pak2-compressed-file",
        "leaf/pak2-image",
    };
}

static auto dummy = fmt::register_fmt<Pak2ArchiveDecoder>("leaf/pak2");
