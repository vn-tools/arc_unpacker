#include "dec/lilim/dpk_archive_decoder.h"
#include "algo/range.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::lilim;

static const bstr magic = "PA"_b;

bool DpkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    input_file.stream.skip(2);
    return input_file.stream.read_le<u32>() == input_file.stream.size();
}

std::unique_ptr<dec::ArchiveMeta> DpkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u16>();
    input_file.stream.skip(4);
    auto meta = std::make_unique<ArchiveMeta>();
    size_t current_offset = magic.size() + 2 + 4 + file_count * 0x14;
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = input_file.stream.read_to_zero(0x10).str();
        entry->size = input_file.stream.read_le<u32>();
        entry->offset = current_offset;
        current_offset += entry->size;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> DpkArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> DpkArchiveDecoder::get_linked_formats() const
{
    return {"lilim/dbm", "lilim/doj", "lilim/dwv"};
}

static auto _ = dec::register_decoder<DpkArchiveDecoder>("lilim/dpk");
