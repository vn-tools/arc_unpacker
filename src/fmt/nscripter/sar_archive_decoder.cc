#include "fmt/nscripter/sar_archive_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nscripter;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool SarArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.has_extension("sar");
}

std::unique_ptr<fmt::ArchiveMeta>
    SarArchiveDecoder::read_meta_impl(File &arc_file) const
{
    u16 file_count = arc_file.stream.read_u16_be();
    u32 offset_to_data = arc_file.stream.read_u32_be();
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.stream.read_to_zero().str();
        entry->offset = arc_file.stream.read_u32_be() + offset_to_data;
        entry->size = arc_file.stream.read_u32_be();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> SarArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.stream.seek(entry->offset);
    auto data = arc_file.stream.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::register_fmt<SarArchiveDecoder>("nscripter/sar");
