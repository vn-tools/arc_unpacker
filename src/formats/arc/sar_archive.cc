// SAR archive
//
// Company:   -
// Engine:    Nscripter
// Extension: .sar
//
// Known games:
// - Tsukihime

#include "formats/arc/sar_archive.h"

namespace
{
    typedef struct
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
    } TableEntry;

    std::unique_ptr<File> read_file(IO &arc_io, const TableEntry &table_entry)
    {
        std::unique_ptr<File> file(new File);
        file->name = table_entry.name;

        arc_io.seek(table_entry.offset);
        file->io.write_from_io(arc_io, table_entry.size);

        return file;
    }
}

void SarArchive::unpack_internal(File &file, FileSaver &file_saver) const
{
    uint16_t file_count = file.io.read_u16_be();
    uint32_t offset_to_files = file.io.read_u32_be();
    if (offset_to_files > file.io.size())
        throw std::runtime_error("Bad offset to files");

    std::vector<std::unique_ptr<TableEntry>> table;
    table.reserve(file_count);
    for (size_t i = 0; i < file_count; i ++)
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = file.io.read_until_zero();
        entry->offset = file.io.read_u32_be() + offset_to_files;
        entry->size = file.io.read_u32_be();
        if (entry->offset + entry->size > file.io.size())
            throw std::runtime_error("Bad offset to file");
        table.push_back(std::move(entry));
    }

    for (auto &table_entry : table)
        file_saver.save(read_file(file.io, *table_entry));
}
