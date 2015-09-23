#include "fmt/fc01/mrg_archive.h"
#include "err.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fc01;

static const bstr magic = "MRG\x00"_b;

namespace
{
    struct TableEntry final
    {
        std::string name;
        u32 offset;
        u32 size_orig;
        u32 size_comp;
        u8 compression_type;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static u8 rol8(u8 x, size_t n)
{
    n &= 7;
    return (x << n) | (x >> (8 - n));
}

static u8 guess_key(const bstr &table_data, size_t file_size)
{
    u8 tmp = rol8(table_data.get<u8>()[table_data.size() - 1], 1);
    u8 key = tmp ^ (file_size >> 24);
    u32 pos = 1;
    u32 last_offset = tmp ^ key;
    for (auto i = table_data.size() - 2; i >= table_data.size() - 4; --i)
    {
        key -= ++pos;
        tmp = rol8(table_data.get<u8>()[i], 1);
        last_offset = (last_offset << 8) | (tmp ^ key);
    }
    if (last_offset != file_size)
        throw err::NotSupportedError("Failed to guess the key");
    while (pos++ < table_data.size())
        key -= pos;
    return key;
}

static Table read_table(io::IO &arc_io)
{
    arc_io.skip(4);
    auto table_size = arc_io.read_u32_le() - 12 - magic.size();
    auto file_count = arc_io.read_u32_le();

    auto table_data = arc_io.read(table_size);
    auto key = guess_key(table_data, arc_io.size());
    for (auto i : util::range(table_data.size()))
    {
        table_data[i] = rol8(table_data[i], 1) ^ key;
        key += table_data.size() - i;
    }

    io::BufferedIO table_io(table_data);
    Table table(file_count);
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = table_io.read_to_zero(0x0E).str();
        entry->size_orig = table_io.read_u32_le();
        entry->compression_type = table_io.read_u8();
        table_io.skip(9);
        entry->offset = table_io.read_u32_le();
        table[i] = std::move(entry);
    }

    table_io.seek(0x20);
    for (size_t i : util::range(file_count))
    {
        table_io.skip(0x1C);
        table[i]->size_comp = table_io.read_u32_le() - table[i]->offset;
    }

    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size_comp);
    if (entry.compression_type != 0)
        Log.err("Compressed MRG is unsupported yet\n");
    file->io.write(data);
    file->name = entry.name;
    return file;
}

bool MrgArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void MrgArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<MrgArchive>("fc01/mrg");
