// YKC archive
//
// Company:   -
// Engine:    YukaScript
// Extension: .ykc
//
// Known games:
// - Hoshizora e Kakaru Hashi

#include "formats/yukascript/ykc_archive.h"
#include "formats/yukascript/ykg_converter.h"
using namespace Formats::YukaScript;

namespace
{
    const std::string magic("YKC001", 6);

    typedef struct
    {
        std::string name;
        size_t size;
        size_t offset;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    Table read_table(IO &arc_io, size_t table_offset, size_t table_size)
    {
        Table table;
        size_t file_count = table_size / 20;

        for (size_t i = 0; i < file_count; i++)
        {
            std::unique_ptr<TableEntry> table_entry(new TableEntry);

            arc_io.seek(table_offset + i * 20);
            size_t name_origin = arc_io.read_u32_le();
            size_t name_size = arc_io.read_u32_le();
            table_entry->offset = arc_io.read_u32_le();
            table_entry->size = arc_io.read_u32_le();
            arc_io.skip(4);

            arc_io.seek(name_origin);
            table_entry->name = arc_io.read(name_size);
            table.push_back(std::move(table_entry));
        }

        return table;
    }

    std::unique_ptr<File> read_file(IO &arc_io, const TableEntry &table_entry)
    {
        std::unique_ptr<File> file(new File);
        arc_io.seek(table_entry.offset);
        file->io.write_from_io(arc_io, table_entry.size);
        file->name = table_entry.name;

        return file;
    }
}

struct YkcArchive::Internals
{
    YkgConverter ykg_converter;
};

YkcArchive::YkcArchive() : internals(new Internals)
{
    add_transformer(&internals->ykg_converter);
}

YkcArchive::~YkcArchive()
{
}

bool YkcArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void YkcArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());
    arc_file.io.skip(2);
    int version = arc_file.io.read_u32_le();
    arc_file.io.skip(4);

    size_t table_offset = arc_file.io.read_u32_le();
    size_t table_size = arc_file.io.read_u32_le();
    Table table = read_table(arc_file.io, table_offset, table_size);

    for (auto &table_entry : table)
        file_saver.save(read_file(arc_file.io, *table_entry));
}
