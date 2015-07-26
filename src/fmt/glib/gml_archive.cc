// GML archive
//
// Company:   Rune
// Engine:    GLib
// Extension: .g, .xp
//
// Known games:
// - Watashi no Puni Puni

#include "fmt/glib/gml_archive.h"
#include "fmt/glib/gml_decoder.h"
#include "fmt/glib/pgx_converter.h"

using namespace au;
using namespace au::fmt::glib;

static const size_t prefix_size = 4;

namespace
{
    typedef struct
    {
        std::string name;
        u32 offset;
        u32 size;
        char prefix[prefix_size];
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;
}

static const std::string magic = "GML_ARC\x00"_s;

static std::unique_ptr<io::BufferedIO> get_header_io(
    io::IO &arc_io, size_t header_size_compressed, size_t header_size_original)
{
    io::BufferedIO temp_io(arc_io, header_size_compressed);
    u8 *buffer = reinterpret_cast<u8*>(temp_io.buffer());
    for (size_t i = 0; i < header_size_compressed; i++)
        buffer[i] ^= 0xff;

    std::unique_ptr<io::BufferedIO> header_io(new io::BufferedIO);
    header_io->reserve(header_size_original);
    GmlDecoder::decode(temp_io, *header_io);
    header_io->seek(0);
    return header_io;
}

static Table read_table(io::IO &table_io, size_t file_data_start)
{
    size_t file_count = table_io.read_u32_le();
    Table table;
    table.reserve(file_count);
    for (size_t i = 0; i < file_count; i++)
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

static std::unique_ptr<File> read_file(
    io::IO &arc_io, const TableEntry &entry, const std::string &permutation)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;

    arc_io.seek(entry.offset);
    io::BufferedIO temp_io(arc_io, entry.size);
    u8 *buffer = reinterpret_cast<u8*>(temp_io.buffer());
    for (size_t i = 0; i < entry.size; i++)
        buffer[i] = permutation[buffer[i]];

    temp_io.skip(prefix_size);
    file->io.write(entry.prefix, prefix_size);
    file->io.write_from_io(temp_io);
    return file;
}

struct GmlArchive::Priv
{
    PgxConverter pgx_converter;
};

GmlArchive::GmlArchive() : p(new Priv)
{
    add_transformer(&p->pgx_converter);
}

GmlArchive::~GmlArchive()
{
}

bool GmlArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void GmlArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());

    u32 file_data_start = arc_file.io.read_u32_le();
    u32 header_size_original = arc_file.io.read_u32_le();
    u32 header_size_compressed = arc_file.io.read_u32_le();

    auto header_io = get_header_io(
        arc_file.io, header_size_compressed, header_size_original);

    std::string permutation = header_io->read(0x100);
    Table table = read_table(*header_io, file_data_start);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry, permutation));
}
