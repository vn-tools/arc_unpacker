#include "fmt/gs/dat_archive_decoder.h"
#include "io/memory_stream.h"
#include "util/format.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::gs;

static const bstr magic = "GsSYMBOL5BINDATA"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size_comp;
        size_t size_orig;
    };
}

bool DatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    DatArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(0xA8);
    auto file_count = input_file.stream.read_u32_le();
    input_file.stream.skip(12);
    auto table_offset = input_file.stream.read_u32_le();
    auto table_size_comp = input_file.stream.read_u32_le();
    auto key = input_file.stream.read_u32_le();
    auto table_size_orig = input_file.stream.read_u32_le();
    auto data_offset = input_file.stream.read_u32_le();

    input_file.stream.seek(table_offset);
    auto table_data = input_file.stream.read(table_size_comp);
    for (auto i : util::range(table_data.size()))
        table_data[i] ^= i & key;
    table_data = util::pack::lzss_decompress_bytewise(
        table_data, table_size_orig);
    io::MemoryStream table_stream(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        table_stream.seek(i * 0x18);
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = util::format("%05d.dat", i);
        entry->offset = table_stream.read_u32_le() + data_offset;
        entry->size_comp = table_stream.read_u32_le();
        entry->size_orig = table_stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> DatArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size_comp);
    data = util::pack::lzss_decompress_bytewise(data, entry->size_orig);
    auto output_file = std::make_unique<io::File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<DatArchiveDecoder>("gs/dat");
