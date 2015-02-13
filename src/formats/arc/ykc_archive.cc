// YKC archive
//
// Company:   -
// Engine:    YukaScript
// Extension: .ykc
//
// Known games:
// - Hoshizora e Kakaru Hashi

#include "formats/arc/ykc_archive.h"
#include "formats/gfx/ykg_converter.h"

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

        for (size_t i = 0; i < file_count; i ++)
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

    std::unique_ptr<VirtualFile> read_file(
        IO &arc_io,
        TableEntry &table_entry,
        YkgConverter &ykg_converter)
    {
        std::unique_ptr<VirtualFile> file(new VirtualFile);
        arc_io.seek(table_entry.offset);
        file->io.write_from_io(arc_io, table_entry.size);
        file->name = table_entry.name;

        ykg_converter.try_decode(*file);
        // todo: decode with YKS once implemented

        return file;
    }
}

struct YkcArchive::Internals
{
    YkgConverter ykg_converter;
    // todo: YKS converters goes here
};

YkcArchive::YkcArchive() : internals(new Internals())
{
}

YkcArchive::~YkcArchive()
{
}

void YkcArchive::add_cli_help(ArgParser &arg_parser) const
{
    internals->ykg_converter.add_cli_help(arg_parser);
    // todo: pass to YKS once implemented
}

void YkcArchive::parse_cli_options(ArgParser &arg_parser)
{
    internals->ykg_converter.parse_cli_options(arg_parser);
    // todo: pass to YKS once implemented
}

void YkcArchive::unpack_internal(
    VirtualFile &file, OutputFiles &output_files) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a YKC archive");

    file.io.skip(2);
    int version = file.io.read_u32_le();
    file.io.skip(4);

    size_t table_offset = file.io.read_u32_le();
    size_t table_size = file.io.read_u32_le();
    Table table = read_table(file.io, table_offset, table_size);

    for (auto &table_entry : table)
    {
        output_files.save([&]() -> std::unique_ptr<VirtualFile>
        {
            return read_file(
                file.io,
                *table_entry,
                internals->ykg_converter);
        });
    }
}
