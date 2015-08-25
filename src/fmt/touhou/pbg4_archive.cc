// PBG4 archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .dat
//
// Known games:
// - Touhou 07 - Perfect Cherry Blossom

#include "fmt/touhou/anm_archive.h"
#include "fmt/touhou/pbg4_archive.h"
#include "io/buffered_io.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

namespace
{
    struct Header
    {
        size_t file_count;
        size_t table_offset;
        size_t table_size;
    };

    struct TableEntry
    {
        size_t size;
        size_t offset;
        std::string name;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static const bstr magic = "PBG4"_b;

static std::unique_ptr<Header> read_header(io::IO &arc_io)
{
    std::unique_ptr<Header> header(new Header);
    header->file_count = arc_io.read_u32_le();
    header->table_offset = arc_io.read_u32_le();
    header->table_size = arc_io.read_u32_le();
    return header;
}

static bstr decompress(io::IO &arc_io, size_t size_original)
{
    util::pack::LzssSettings settings;
    settings.position_bits = 13;
    settings.size_bits = 4;
    settings.min_match_size = 3;
    settings.initial_dictionary_pos = 1;
    io::BitReader bit_reader(arc_io);
    return util::pack::lzss_decompress(bit_reader, size_original, settings);
}

static Table read_table(io::IO &arc_io, Header &header)
{
    Table table;
    arc_io.seek(header.table_offset);

    io::BufferedIO table_io(decompress(arc_io, header.table_size));
    for (auto i : util::range(header.file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = table_io.read_to_zero().str();
        entry->offset = table_io.read_u32_le();
        entry->size = table_io.read_u32_le();
        table_io.skip(4);
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    arc_io.seek(entry.offset);
    file->io.write(decompress(arc_io, entry.size));
    file->name = entry.name;
    return file;
}

struct Pbg4Archive::Priv
{
    AnmArchive anm_archive;
};

Pbg4Archive::Pbg4Archive() : p(new Priv)
{
    add_transformer(&p->anm_archive);
}

Pbg4Archive::~Pbg4Archive()
{
}

bool Pbg4Archive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void Pbg4Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());

    // works much faster when the whole archive resides in memory
    arc_file.io.seek(0);
    io::BufferedIO buf_io;
    buf_io.write_from_io(arc_file.io, arc_file.io.size());
    buf_io.seek(magic.size());

    auto header = read_header(buf_io);
    auto table = read_table(buf_io, *header);

    for (auto &entry : table)
        file_saver.save(read_file(buf_io, *entry));
}
