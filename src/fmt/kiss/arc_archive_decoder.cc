#include "fmt/kiss/arc_archive_decoder.h"
#include "err.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::kiss;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool ArcArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    if (!arc_file.has_extension("arc"))
        return false;
    const auto meta = read_meta_impl(arc_file);
    if (meta->entries.empty())
        return false;
    const auto last_entry
        = dynamic_cast<ArchiveEntryImpl*>(meta->entries.back().get());
    return last_entry->size + last_entry->offset == arc_file.stream.size();
}

std::unique_ptr<fmt::ArchiveMeta>
    ArcArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.stream.seek(0);
    const auto file_count = arc_file.stream.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    ArchiveEntryImpl *last_entry = nullptr;
    for (const size_t i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = util::sjis_to_utf8(arc_file.stream.read_to_zero()).str();
        entry->offset = arc_file.stream.read_u32_le();
        if (arc_file.stream.read_u32_le() != 0)
            throw err::CorruptDataError("Expected '0'");
        if (last_entry)
            last_entry->size = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }
    if (last_entry)
        last_entry->size = arc_file.stream.size() - last_entry->offset;
    return meta;
}

std::unique_ptr<File> ArcArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = arc_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> ArcArchiveDecoder::get_linked_formats() const
{
    return {"kiss/plg"};
}

static auto dummy = fmt::register_fmt<ArcArchiveDecoder>("kiss/arc");
