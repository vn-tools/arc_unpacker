#include "fmt/real_live/ovk_archive_decoder.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::real_live;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool OvkArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.has_extension("ovk");
}

std::unique_ptr<fmt::ArchiveMeta>
    OvkArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto file_count = arc_file.stream.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->size = arc_file.stream.read_u32_le();
        entry->offset = arc_file.stream.read_u32_le();
        entry->name = util::format("sample%05d", arc_file.stream.read_u32_le());
        arc_file.stream.skip(4);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> OvkArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.stream.seek(entry->offset);
    auto data = arc_file.stream.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<OvkArchiveDecoder>("real-live/ovk");
