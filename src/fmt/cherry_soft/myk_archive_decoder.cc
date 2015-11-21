#include "fmt/cherry_soft/myk_archive_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::cherry_soft;

static const bstr magic = "MYK00\x1A\x00\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool MykArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    MykArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.stream.seek(magic.size());
    auto file_count = arc_file.stream.read_u16_le();
    auto table_offset = arc_file.stream.read_u32_le();
    arc_file.stream.seek(table_offset);

    auto meta = std::make_unique<ArchiveMeta>();
    auto current_offset = 16;
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.stream.read_to_zero(12).str();
        entry->offset = current_offset;
        entry->size = arc_file.stream.read_u32_le();
        current_offset += entry->size;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> MykArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.stream.seek(entry->offset);
    auto data = arc_file.stream.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::register_fmt<MykArchiveDecoder>("cherry-soft/myk");
