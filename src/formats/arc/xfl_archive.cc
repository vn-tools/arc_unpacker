// XFL archive
//
// Company:   Liar-soft
// Engine:    -
// Extension: .xfl
//
// Known games:
// - Souten No Celenaria - What a Beautiful World

#include "formats/arc/xfl_archive.h"

namespace
{
    const std::string magic("LB\x01\x00", 4);

    typedef struct
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    Table read_table(IO &arc_io)
    {
        Table table;
        size_t table_size = arc_io.read_u32_le();
        size_t file_count = arc_io.read_u32_le();
        size_t file_start = arc_io.tell() + table_size;
        table.reserve(file_count);
        for (size_t i = 0; i < file_count; i ++)
        {
            std::unique_ptr<TableEntry> table_entry(new TableEntry);
            table_entry->name = arc_io.read_until_zero(0x20);
            table_entry->offset = file_start + arc_io.read_u32_le();
            table_entry->size = arc_io.read_u32_le();
            table.push_back(std::move(table_entry));
        }
        return table;
    }

    std::unique_ptr<File> read_file(IO &arc_io, const TableEntry &table_entry)
    {
        std::unique_ptr<File> file(new File);
        file->name = table_entry.name;
        arc_io.seek(table_entry.offset);
        file->io.write_from_io(arc_io, table_entry.size);
        return file;
    }
}

void XflArchive::unpack_internal(File &file, FileSaver &file_saver) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not an XFL archive");

    Table table = read_table(file.io);
    for (auto &table_entry : table)
        file_saver.save(read_file(file.io, *table_entry));
}
