#include "fmt/gs/pak_archive_decoder.h"
#include "io/memory_stream.h"
#include "util/pack/lzss.h"
#include "util/range.h"

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

bool PakArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    PakArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.stream.seek(0x30);
    auto version = arc_file.stream.read_u32_le();
    auto entry_size = (version >> 16) < 5 ? 0x48 : 0x68;

    auto table_size_comp = arc_file.stream.read_u32_le();
    auto key = arc_file.stream.read_u32_le();
    auto file_count = arc_file.stream.read_u32_le();
    auto data_offset = arc_file.stream.read_u32_le();
    auto table_offset = arc_file.stream.read_u32_le();

    auto table_size_orig = file_count * entry_size;

    arc_file.stream.seek(table_offset);
    auto table_data = arc_file.stream.read(table_size_comp);
    for (auto i : util::range(table_data.size()))
        table_data[i] ^= i & key;
    table_data = util::pack::lzss_decompress_bytewise(
        table_data, table_size_orig);
    io::MemoryStream table_stream(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        table_stream.seek(entry_size * i);
        entry->name = table_stream.read_to_zero(0x40).str();
        entry->offset = table_stream.read_u32_le() + data_offset;
        entry->size = table_stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> PakArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.stream.seek(entry->offset);
    auto data = arc_file.stream.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> PakArchiveDecoder::get_linked_formats() const
{
    return {"gs/gfx"};
}

static auto dummy = fmt::register_fmt<PakArchiveDecoder>("gs/pak");
