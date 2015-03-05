// LWG archive
//
// Company:   Liar-soft
// Engine:    -
// Extension: .lwg
//
// Known games:
// - Souten No Celenaria - What a Beautiful World

#include "formats/liarsoft/lwg_archive.h"
#include "formats/liarsoft/wcg_converter.h"
#include "util/encoding.h"
using namespace Formats::LiarSoft;

namespace
{
    const std::string magic("LG\x01\x00", 4);

    typedef struct
    {
        std::string name;
        size_t offset;
        size_t size;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    Table read_table(IO &arc_io)
    {
        size_t file_count = arc_io.read_u32_le();
        arc_io.skip(4);
        size_t table_size = arc_io.read_u32_le();
        size_t file_start = arc_io.tell() + table_size + 4;

        Table table;
        table.reserve(file_count);
        for (size_t i = 0; i < file_count; i ++)
        {
            std::unique_ptr<TableEntry> table_entry(new TableEntry);
            arc_io.skip(9);
            table_entry->offset = file_start + arc_io.read_u32_le();
            table_entry->size = arc_io.read_u32_le();
            table_entry->name = convert_encoding(
                arc_io.read(arc_io.read_u8()), "cp932", "utf-8");
            table.push_back(std::move(table_entry));
        }
        size_t file_data_size = arc_io.read_u32_le();
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


struct LwgArchive::Internals
{
    WcgConverter wcg_converter;
};

LwgArchive::LwgArchive() : internals(new Internals)
{
}

LwgArchive::~LwgArchive()
{
}

void LwgArchive::add_cli_help(ArgParser &arg_parser) const
{
    internals->wcg_converter.add_cli_help(arg_parser);
}

void LwgArchive::parse_cli_options(ArgParser &arg_parser)
{
    internals->wcg_converter.parse_cli_options(arg_parser);
}

void LwgArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    if (arc_file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not an LWG archive");
    size_t image_width = arc_file.io.read_u32_le();
    size_t image_height = arc_file.io.read_u32_le();

    Table table = read_table(arc_file.io);
    for (auto &table_entry : table)
    {
        auto file = read_file(arc_file.io, *table_entry);
        internals->wcg_converter.try_decode(*file);
        file_saver.save(std::move(file));
    }
}
