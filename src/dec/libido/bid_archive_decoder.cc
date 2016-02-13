#include "dec/libido/bid_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::libido;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool BidArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    auto data_start = input_file.stream.read_le<u32>();
    input_file.stream.seek(data_start - 8);
    auto last_file_offset = input_file.stream.read_le<u32>() + data_start;
    auto last_file_size = input_file.stream.read_le<u32>();
    return last_file_offset + last_file_size == input_file.stream.size();
}

std::unique_ptr<dec::ArchiveMeta> BidArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    u32 data_start = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);
    while (input_file.stream.pos() < data_start)
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = input_file.stream.read_to_zero(16).str();
        entry->offset = input_file.stream.read_le<u32>() + data_start;
        entry->size = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> BidArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> BidArchiveDecoder::get_linked_formats() const
{
    return {"libido/mnc"};
}

static auto _ = dec::register_decoder<BidArchiveDecoder>("libido/bid");
