// PAK archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .pak
//
// Known games:
// - Touhou 07.5 - Immaterial and Missing Power

#include "buffered_io.h"
#include "formats/touhou/pak_archive.h"
using namespace Formats::Touhou;

namespace
{
    typedef struct
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    std::unique_ptr<File> read_file(IO &arc_io, const TableEntry &table_entry)
    {
        std::unique_ptr<File> file(new File);
        arc_io.seek(table_entry.offset);
        file->io.write_from_io(arc_io, table_entry.size);
        file->name = table_entry.name;
        return file;
    }

    std::unique_ptr<BufferedIO> read_raw_table(IO &arc_io, size_t file_count)
    {
        size_t table_size = file_count * 0x6c;
        std::unique_ptr<char[]> table(new char[table_size]);
        arc_io.read(table.get(), table_size);

        uint8_t k = 0x64;
        uint8_t t = 0x64;
        for (size_t i = 0; i < table_size; i ++)
        {
            table[i] ^= k;
            k += t;
            t += 0x4d;
        }
        return std::unique_ptr<BufferedIO>(
            new BufferedIO(table.get(), table_size));
    }

    Table read_table(IO &arc_io)
    {
        uint16_t file_count = arc_io.read_u16_le();
        auto table_io = read_raw_table(arc_io, file_count);
        Table table;
        table.reserve(file_count);
        for (size_t i = 0; i < file_count; i ++)
        {
            std::unique_ptr<TableEntry> entry(new TableEntry);
            entry->name = table_io->read_until_zero(0x64);
            entry->size = table_io->read_u32_le();
            entry->offset = table_io->read_u32_le();
            if (entry->offset + entry->size > arc_io.size())
                throw std::runtime_error("Bad offset to file");
            table.push_back(std::move(entry));
        }
        return table;
    }
}

void PakArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto table = read_table(arc_file.io);
    for (auto &table_entry : table)
        file_saver.save(read_file(arc_file.io, *table_entry));
}
