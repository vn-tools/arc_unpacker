#include "fmt/amuse_craft/pac_archive_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::amuse_craft;

static const bstr magic = "PAC\x20"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool PacArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    PacArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    arc_file.io.skip(4);
    const auto file_count = arc_file.io.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    arc_file.io.seek(0x804);
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.io.read_to_zero(32).str();
        std::replace(entry->name.begin(), entry->name.end(), '_', '/');
        entry->size = arc_file.io.read_u32_le();
        entry->offset = arc_file.io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> PacArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> PacArchiveDecoder::get_linked_formats() const
{
    return { "amuse-craft/pgd" };
}

static auto dummy = fmt::register_fmt<PacArchiveDecoder>("amuse-craft/pac");
