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
    struct TableEntry final
    {
        std::string name;
        size_t offset;
        size_t size_comp;
        size_t size_orig;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_table(io::IO &arc_io)
{
    arc_io.seek(0xA8);
    auto file_count = arc_io.read_u32_le();
    arc_io.skip(12);
    auto table_offset = arc_io.read_u32_le();
    auto table_size_comp = arc_io.read_u32_le();
    auto key = arc_io.read_u32_le();
    auto table_size_orig = arc_io.read_u32_le();
    auto data_offset = arc_io.read_u32_le();

    arc_io.seek(table_offset);
    auto table_data = arc_io.read(table_size_comp);
    for (auto i : util::range(table_data.size()))
        table_data[i] ^= i & key;
    table_data = util::pack::lzss_decompress_bytewise(
        table_data, table_size_orig);
    io::BufferedIO table_io(table_data);

    Table table;
    for (auto i : util::range(file_count))
    {
        table_io.seek(i * 0x18);
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = util::format("%05d.dat", i);
        entry->offset = table_io.read_u32_le() + data_offset;
        entry->size_comp = table_io.read_u32_le();
        entry->size_orig = table_io.read_u32_le();
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size_comp);
    data = util::pack::lzss_decompress_bytewise(data, entry.size_orig);

    std::unique_ptr<File> file(new File);
    file->name = entry.name;
    file->io.write(data);
    return file;
}

bool DatArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void DatArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    arc_file.io.skip(magic.size());
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<DatArchiveDecoder>("gs/dat");
