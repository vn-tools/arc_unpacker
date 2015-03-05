// PAK2 archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .pak
//
// Known games:
// - Touhou 10.5 - Scarlet Weather Rhapsody

#include "buffered_io.h"
#include "formats/touhou/pak2_archive.h"
#include "util/mt.h"
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

    void decrypt(IO &io, uint32_t mt_seed, uint8_t a, uint8_t b, uint8_t delta)
    {
        size_t size = io.size();
        std::unique_ptr<char[]> buffer(new char[size]);
        io.seek(0);
        io.read(buffer.get(), size);
        mt_init_genrand(mt_seed);
        for (size_t i = 0; i < size; i ++)
        {
            buffer[i] ^= mt_genrand_int32();
            buffer[i] ^= a;
            a += b;
            b += delta;
        }
        io.seek(0);
        io.write(buffer.get(), size);
        io.seek(0);
    }

    std::unique_ptr<File> read_file(IO &arc_io, const TableEntry &table_entry)
    {
        std::unique_ptr<char[]> data(new char[table_entry.size]);
        arc_io.seek(table_entry.offset);
        arc_io.read(data.get(), table_entry.size);

        uint8_t key = (table_entry.offset >> 1) | 0x23;
        for (size_t i = 0; i < table_entry.size; i ++)
            data[i] ^= key;

        std::unique_ptr<File> file(new File);
        file->io.write(data.get(), table_entry.size);
        file->name = table_entry.name;
        return file;
    }

    std::unique_ptr<BufferedIO> read_raw_table(IO &arc_io, size_t file_count)
    {
        size_t table_size = arc_io.read_u32_le();
        std::unique_ptr<BufferedIO> table_io(new BufferedIO());
        table_io->write_from_io(arc_io, table_size);
        decrypt(*table_io, table_size + 6, 0xc5, 0x83, 0x53);
        return table_io;
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
            entry->offset = table_io->read_u32_le();
            entry->size = table_io->read_u32_le();
            entry->name = table_io->read(table_io->read_u8());
            if (entry->offset + entry->size > arc_io.size())
                throw std::runtime_error("Bad offset to file");
            table.push_back(std::move(entry));
        }
        return table;
    }
}

void Pak2Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto table = read_table(arc_file.io);
    for (auto &table_entry : table)
        file_saver.save(read_file(arc_file.io, *table_entry));
}
