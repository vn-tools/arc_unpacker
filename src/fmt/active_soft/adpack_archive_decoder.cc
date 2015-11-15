#include "fmt/active_soft/adpack_archive_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::active_soft;

static const bstr magic = "ADPACK32"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool AdpackArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    AdpackArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size() + 4);
    const auto file_count = arc_file.io.read_u32_le() - 1;
    arc_file.io.seek(0x10);
    ArchiveEntryImpl *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.io.read_to_zero(0x18).str();
        arc_file.io.skip(4);
        entry->offset = arc_file.io.read_u32_le();
        if (last_entry)
            last_entry->size = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }
    if (last_entry)
        last_entry->size = arc_file.io.size() - last_entry->offset;
    return meta;
}

std::unique_ptr<File> AdpackArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    return std::make_unique<File>(entry->name, arc_file.io.read(entry->size));
}

std::vector<std::string> AdpackArchiveDecoder::get_linked_formats() const
{
    return {"active-soft/ed8", "active-soft/edt"};
}

static auto dummy
    = fmt::register_fmt<AdpackArchiveDecoder>("active-soft/adpack");
