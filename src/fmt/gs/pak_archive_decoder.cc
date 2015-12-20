#include "fmt/gs/pak_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::fmt::gs;

static const bstr magic = "DataPack5\x00\x00\x00\x00\x00\x00\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool PakArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    PakArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(0x30);
    auto version = input_file.stream.read_u32_le();
    auto entry_size = (version >> 16) < 5 ? 0x48 : 0x68;

    auto table_size_comp = input_file.stream.read_u32_le();
    auto key = input_file.stream.read_u32_le();
    auto file_count = input_file.stream.read_u32_le();
    auto data_offset = input_file.stream.read_u32_le();
    auto table_offset = input_file.stream.read_u32_le();

    auto table_size_orig = file_count * entry_size;

    input_file.stream.seek(table_offset);
    auto table_data = input_file.stream.read(table_size_comp);
    for (auto i : algo::range(table_data.size()))
        table_data[i] ^= i & key;
    table_data = algo::pack::lzss_decompress_bytewise(
        table_data, table_size_orig);
    io::MemoryStream table_stream(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        table_stream.seek(entry_size * i);
        entry->path = table_stream.read_to_zero(0x40).str();
        entry->offset = table_stream.read_u32_le() + data_offset;
        entry->size = table_stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> PakArchiveDecoder::read_file_impl(
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> PakArchiveDecoder::get_linked_formats() const
{
    return {"gs/gfx"};
}

static auto dummy = fmt::register_fmt<PakArchiveDecoder>("gs/pak");
