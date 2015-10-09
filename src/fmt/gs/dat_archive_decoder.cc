#include "fmt/gs/dat_archive_decoder.h"
#include "io/buffered_io.h"
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

bool DatArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    DatArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(0xA8);
    auto file_count = arc_file.io.read_u32_le();
    arc_file.io.skip(12);
    auto table_offset = arc_file.io.read_u32_le();
    auto table_size_comp = arc_file.io.read_u32_le();
    auto key = arc_file.io.read_u32_le();
    auto table_size_orig = arc_file.io.read_u32_le();
    auto data_offset = arc_file.io.read_u32_le();

    arc_file.io.seek(table_offset);
    auto table_data = arc_file.io.read(table_size_comp);
    for (auto i : util::range(table_data.size()))
        table_data[i] ^= i & key;
    table_data = util::pack::lzss_decompress_bytewise(
        table_data, table_size_orig);
    io::BufferedIO table_io(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        table_io.seek(i * 0x18);
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = util::format("%05d.dat", i);
        entry->offset = table_io.read_u32_le() + data_offset;
        entry->size_comp = table_io.read_u32_le();
        entry->size_orig = table_io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> DatArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size_comp);
    data = util::pack::lzss_decompress_bytewise(data, entry->size_orig);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::Registry::add<DatArchiveDecoder>("gs/dat");
