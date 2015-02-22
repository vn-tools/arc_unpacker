// NPA archive
//
// Company:   Nitroplus
// Engine:    -
// Extension: .npa
//
// Known games:
// - Steins;Gate

#include "buffered_io.h"
#include "formats/arc/npa_sg_archive.h"
#include "string/encoding.h"

namespace
{
    const std::string key("\xBD\xAA\xBC\xB4\xAB\xB6\xBC\xB4", 8);

    typedef struct
    {
        std::string name;
        size_t offset;
        size_t size;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    void decrypt(char *data, size_t data_size)
    {
        for (size_t i = 0; i < data_size; i ++)
            data[i] ^= key[i % key.length()];
    }

    Table read_table(IO &table_io, const IO &arc_io)
    {
        Table table;
        size_t file_count = table_io.read_u32_le();
        for (size_t i = 0; i < file_count; i ++)
        {
            std::unique_ptr<TableEntry> table_entry(new TableEntry);
            table_entry->name = convert_encoding(
                table_io.read(table_io.read_u32_le()),
                "utf-16le",
                "utf-8");
            table_entry->size = table_io.read_u32_le();
            table_entry->offset = table_io.read_u32_le();
            table_io.skip(4);
            if (table_entry->offset + table_entry->size > arc_io.size())
                throw std::runtime_error("Bad offset to file");
            table.push_back(std::move(table_entry));
        }
        return table;
    }

    std::unique_ptr<File> read_file(IO &arc_io, TableEntry &table_entry)
    {
        std::unique_ptr<File> file(new File);
        std::unique_ptr<char[]> data(new char[table_entry.size]);
        arc_io.seek(table_entry.offset);
        arc_io.read(data.get(), table_entry.size);
        decrypt(data.get(), table_entry.size);
        file->name = table_entry.name;
        file->io.write(data.get(), table_entry.size);
        return file;
    }
}

void NpaSgArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    size_t table_size = arc_file.io.read_u32_le();
    if (table_size > arc_file.io.size())
        throw std::runtime_error("Bad table size");

    std::unique_ptr<char> table_bytes(new char[table_size]);
    arc_file.io.read(table_bytes.get(), table_size);
    decrypt(table_bytes.get(), table_size);

    BufferedIO table_io(table_bytes.get(), table_size);
    Table table = read_table(table_io, arc_file.io);
    for (auto &table_entry : table)
        file_saver.save(read_file(arc_file.io, *table_entry));
}
