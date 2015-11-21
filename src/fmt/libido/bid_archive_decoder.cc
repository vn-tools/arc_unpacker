#include "fmt/libido/bid_archive_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::libido;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool BidArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    auto data_start = arc_file.stream.read_u32_le();
    arc_file.stream.seek(data_start - 8);
    auto last_file_offset = arc_file.stream.read_u32_le() + data_start;
    auto last_file_size = arc_file.stream.read_u32_le();
    return last_file_offset + last_file_size == arc_file.stream.size();
}

std::unique_ptr<fmt::ArchiveMeta>
    BidArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    u32 data_start = arc_file.stream.read_u32_le();
    arc_file.stream.skip(4);
    while (arc_file.stream.tell() < data_start)
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.stream.read_to_zero(16).str();
        entry->offset = arc_file.stream.read_u32_le() + data_start;
        entry->size = arc_file.stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> BidArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.stream.seek(entry->offset);
    auto data = arc_file.stream.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> BidArchiveDecoder::get_linked_formats() const
{
    return {"libido/mnc"};
}

static auto dummy = fmt::register_fmt<BidArchiveDecoder>("libido/bid");
