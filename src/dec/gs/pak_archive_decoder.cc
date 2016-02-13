#include "dec/gs/pak_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::gs;

static const bstr magic = "DataPack5\x00\x00\x00\x00\x00\x00\x00"_b;

bool PakArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> PakArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0x30);
    const auto version = input_file.stream.read_le<u32>();
    const auto entry_size = (version >> 16) < 5 ? 0x48 : 0x68;

    const auto table_size_comp = input_file.stream.read_le<u32>();
    const auto key = input_file.stream.read_le<u32>();
    const auto file_count = input_file.stream.read_le<u32>();
    const auto data_offset = input_file.stream.read_le<u32>();
    const auto table_offset = input_file.stream.read_le<u32>();

    const auto table_size_orig = file_count * entry_size;

    input_file.stream.seek(table_offset);
    auto table_data = input_file.stream.read(table_size_comp);
    for (const auto i : algo::range(table_data.size()))
        table_data[i] ^= i & key;
    table_data = algo::pack::lzss_decompress(table_data, table_size_orig);
    io::MemoryStream table_stream(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        table_stream.seek(entry_size * i);
        entry->path = table_stream.read_to_zero(0x40).str();
        entry->offset = table_stream.read_le<u32>() + data_offset;
        entry->size = table_stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> PakArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> PakArchiveDecoder::get_linked_formats() const
{
    return {"gs/gfx"};
}

static auto _ = dec::register_decoder<PakArchiveDecoder>("gs/pak");
