// YKC archive
//
// Company:   -
// Engine:    YukaScript
// Extension: .ykc
//
// Known games:
// - Hoshizora e Kakaru Hashi

#include "formats/arc/ykc_archive.h"

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

    std::unique_ptr<VirtualFile> read_file(IO &arc_io, TableEntry &table_entry)
    {
        std::unique_ptr<VirtualFile> file(new VirtualFile);
        arc_io.seek(table_entry.offset);
        file->io.write_from_io(arc_io, table_entry.size);
        file->name = table_entry.name;

        // todo: decode with YKG and YKS once implemented

        return file;
    }
}

struct YkcArchive::Internals
{
    // todo: YKG and YKS converters go here
};

YkcArchive::YkcArchive() : internals(new Internals())
{
}

YkcArchive::~YkcArchive()
{
}

void YkcArchive::add_cli_help(
    __attribute__((unused)) ArgParser &arg_parser) const
{
    // todo: pass to YKG and YKS once implemented
}

void YkcArchive::parse_cli_options(
    __attribute__((unused)) ArgParser &arg_parser)
{
    // todo: pass to YKG and YKS once implemented
}

void YkcArchive::unpack_internal(IO &arc_io, OutputFiles &output_files) const
{
    if (arc_io.read(magic.size()) != magic)
        throw std::runtime_error("Not a YKC archive");

    arc_io.skip(2);
    __attribute__((unused)) int version = arc_io.read_u32_le();
    arc_io.skip(4);

    size_t table_offset = arc_io.read_u32_le();
    size_t table_size = arc_io.read_u32_le();
    Table table = read_table(arc_io, table_offset, table_size);

    for (auto &table_entry : table)
    {
        output_files.save([&]() -> std::unique_ptr<VirtualFile>
        {
            return read_file(arc_io, *table_entry);
        });
    }
}
