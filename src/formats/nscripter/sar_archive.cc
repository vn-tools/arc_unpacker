// SAR archive
//
// Company:   -
// Engine:    NScripter
// Extension: .sar
//
// Known games:
// - Tsukihime

#include "formats/nscripter/sar_archive.h"
using namespace Formats::NScripter;

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

bool SarArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("sar");
}

void SarArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    uint16_t file_count = arc_file.io.read_u16_be();
    uint32_t offset_to_files = arc_file.io.read_u32_be();

    std::vector<std::unique_ptr<TableEntry>> table;
    table.reserve(file_count);
    for (size_t i = 0; i < file_count; i ++)
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = arc_file.io.read_until_zero();
        entry->offset = arc_file.io.read_u32_be() + offset_to_files;
        entry->size = arc_file.io.read_u32_be();
        table.push_back(std::move(entry));
    }

    for (auto &table_entry : table)
        file_saver.save(read_file(arc_file.io, *table_entry));
}
