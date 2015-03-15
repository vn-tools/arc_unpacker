// GML archive
//
// Company:   Rune
// Engine:    GLib
// Extension: .g, .xp
//
// Known games:
// - Watashi no Puni Puni

#include "formats/glib/gml_archive.h"
#include "formats/glib/gml_decoder.h"
#include "formats/glib/pgx_converter.h"
using namespace Formats::Glib;

namespace
{
    const std::string magic("GML_ARC\x00", 8);
    const size_t prefix_size = 4;

    typedef struct
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
        char prefix[prefix_size];
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    std::unique_ptr<BufferedIO> get_header_io(
        IO &arc_io, size_t header_size_compressed, size_t header_size_original)
    {
        BufferedIO temp_io(arc_io, header_size_compressed);
        uint8_t *buffer = reinterpret_cast<uint8_t*>(temp_io.buffer());
        for (size_t i = 0; i < header_size_compressed; i ++)
            buffer[i] ^= 0xff;

        std::unique_ptr<BufferedIO> header_io(new BufferedIO);
        header_io->reserve(header_size_original);
        GmlDecoder::decode(temp_io, *header_io);
        header_io->seek(0);
        return header_io;
    }

    Table read_table(IO &table_io, size_t file_data_start)
    {
        size_t file_count = table_io.read_u32_le();
        Table table;
        table.reserve(file_count);
        for (size_t i = 0; i < file_count; i ++)
        {
            std::unique_ptr<TableEntry> entry(new TableEntry);
            entry->name = table_io.read(table_io.read_u32_le());
            entry->offset = table_io.read_u32_le() + file_data_start;
            entry->size = table_io.read_u32_le();
            table_io.read(entry->prefix, prefix_size);
            table.push_back(std::move(entry));
        }
        return table;
    }

    std::unique_ptr<File> read_file(
        IO &arc_io,
        const TableEntry &table_entry,
        const std::string &permutation)
    {
        std::unique_ptr<File> file(new File);
        file->name = table_entry.name;

        arc_io.seek(table_entry.offset);
        BufferedIO temp_io(arc_io, table_entry.size);
        uint8_t *buffer = reinterpret_cast<uint8_t*>(temp_io.buffer());
        for (size_t i = 0; i < table_entry.size; i ++)
            buffer[i] = permutation[buffer[i]];

        temp_io.skip(prefix_size);
        file->io.write(table_entry.prefix, prefix_size);
        file->io.write_from_io(temp_io);
        return file;
    }
}

struct GmlArchive::Internals
{
    PgxConverter pgx_converter;
};

GmlArchive::GmlArchive() : internals(new Internals())
{
}

GmlArchive::~GmlArchive()
{
}

void GmlArchive::add_cli_help(ArgParser &arg_parser) const
{
    internals->pgx_converter.add_cli_help(arg_parser);
}

void GmlArchive::parse_cli_options(ArgParser &arg_parser)
{
    internals->pgx_converter.parse_cli_options(arg_parser);
}

void GmlArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    if (arc_file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a GML archive");

    uint32_t file_data_start = arc_file.io.read_u32_le();
    uint32_t header_size_original = arc_file.io.read_u32_le();
    uint32_t header_size_compressed = arc_file.io.read_u32_le();

    auto header_io = get_header_io(
        arc_file.io, header_size_compressed, header_size_original);

    std::string permutation = header_io->read(0x100);
    Table table = read_table(*header_io, file_data_start);
    for (auto &table_entry : table)
    {
        auto file = read_file(arc_file.io, *table_entry, permutation);
        internals->pgx_converter.try_decode(*file);
        file_saver.save(std::move(file));
    }
}
