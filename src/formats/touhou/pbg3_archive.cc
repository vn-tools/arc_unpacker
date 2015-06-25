// PBG3 archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .dat
//
// Known games:
// - Touhou 06 - The Embodiment of Scarlet Devil

#include "formats/touhou/anm_archive.h"
#include "formats/touhou/pbg3_archive.h"
#include "io/buffered_io.h"
#include "util/lzss.h"
using namespace Formats::Touhou;

namespace
{
    typedef struct
    {
        size_t file_count;
        size_t table_offset;
    } Header;

    typedef struct
    {
        u32 checksum;
        size_t size;
        size_t offset;
        std::string name;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;
}

static const std::string magic("PBG3", 4);

static unsigned int read_integer(BitReader &bit_reader)
{
    size_t integer_size = bit_reader.get(2) + 1;
    return bit_reader.get(integer_size << 3);
}

static std::unique_ptr<Header> read_header(IO &arc_io)
{
    std::unique_ptr<Header> header(new Header);
    BitReader bit_reader(arc_io);
    header->file_count = read_integer(bit_reader);
    header->table_offset = read_integer(bit_reader);
    return header;
}

static Table read_table(IO &arc_io, Header &header)
{
    Table table;
    arc_io.seek(header.table_offset);
    BitReader bit_reader(arc_io);
    for (size_t i = 0; i < header.file_count; i++)
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        read_integer(bit_reader);
        read_integer(bit_reader);
        entry->checksum = read_integer(bit_reader);
        entry->offset = read_integer(bit_reader);
        entry->size = read_integer(bit_reader);
        for (size_t i = 0; i < 256; i++)
        {
            char c = static_cast<char>(bit_reader.get(8));
            if (c == 0)
                break;
            entry->name.push_back(c);
        }
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;

    arc_io.seek(entry.offset);
    BitReader bit_reader(arc_io);

    LzssSettings settings;
    settings.position_bits = 13;
    settings.length_bits = 4;
    settings.min_match_length = 3;
    settings.initial_dictionary_pos = 1;
    settings.reuse_compressed = true;
    file->io.write(lzss_decompress(bit_reader, entry.size, settings));

    return file;
}

struct Pbg3Archive::Priv
{
    AnmArchive anm_archive;
};

Pbg3Archive::Pbg3Archive() : p(new Priv)
{
    add_transformer(&p->anm_archive);
}

Pbg3Archive::~Pbg3Archive()
{
}

bool Pbg3Archive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void Pbg3Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());

    // works much faster when the whole archive resides in memory
    arc_file.io.seek(0);
    BufferedIO buf_io;
    buf_io.write_from_io(arc_file.io, arc_file.io.size());
    buf_io.seek(magic.size());

    auto header = read_header(buf_io);
    auto table = read_table(buf_io, *header);

    for (auto &entry : table)
        file_saver.save(read_file(buf_io, *entry));
}
