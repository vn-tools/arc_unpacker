#include "fmt/cri/afs2_archive_decoder.h"
#include "err.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::cri;

static const bstr magic = "AFS2"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool Afs2ArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    arc_file.io.seek(0);
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Afs2ArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size() + 4);
    const auto file_count = arc_file.io.read_u32_le() - 1;
    arc_file.io.skip(4);
    arc_file.io.skip((file_count + 1) * 2);
    ArchiveEntryImpl *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = util::format("%d.dat", i);
        entry->offset = arc_file.io.read_u32_le();
        if (last_entry)
            last_entry->size = entry->offset - last_entry->offset;
        last_entry = entry.get();
        entry->offset = (entry->offset + 0x1F) & (~0x1F);
        meta->entries.push_back(std::move(entry));
    }
    if (last_entry)
        last_entry->size = arc_file.io.size() - last_entry->offset;
    return meta;
}

std::unique_ptr<File> Afs2ArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    return std::make_unique<File>(entry->name, arc_file.io.read(entry->size));
}

std::vector<std::string> Afs2ArchiveDecoder::get_linked_formats() const
{
    return {"cri/hca"};
}

static auto dummy = fmt::register_fmt<Afs2ArchiveDecoder>("cri/afs2");
