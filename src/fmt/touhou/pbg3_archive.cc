// PBG3 archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .dat
//
// Known games:
// - [Team Shanghai Alice] [020811] TH06 - The Embodiment of Scarlet Devil

#include "fmt/touhou/pbg3_archive.h"
#include "fmt/touhou/anm_archive.h"
#include "io/buffered_io.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

namespace
{
    struct Header final
    {
        size_t file_count;
        size_t table_offset;
    };

    struct TableEntry final
    {
        u32 checksum;
        size_t size;
        size_t offset;
        std::string name;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static const bstr magic = "PBG3"_b;

static unsigned int read_integer(io::BitReader &bit_reader)
{
    size_t integer_size = bit_reader.get(2) + 1;
    return bit_reader.get(integer_size << 3);
}

static std::unique_ptr<Header> read_header(io::IO &arc_io)
{
    std::unique_ptr<Header> header(new Header);
    io::BitReader bit_reader(arc_io);
    header->file_count = read_integer(bit_reader);
    header->table_offset = read_integer(bit_reader);
    return header;
}

static Table read_table(io::IO &arc_io, Header &header)
{
    Table table;
    arc_io.seek(header.table_offset);
    io::BitReader bit_reader(arc_io);
    for (auto i : util::range(header.file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        read_integer(bit_reader);
        read_integer(bit_reader);
        entry->checksum = read_integer(bit_reader);
        entry->offset = read_integer(bit_reader);
        entry->size = read_integer(bit_reader);
        for (auto j : util::range(256))
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

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;

    arc_io.seek(entry.offset);
    io::BitReader bit_reader(arc_io);

    util::pack::LzssSettings settings;
    settings.position_bits = 13;
    settings.size_bits = 4;
    settings.min_match_size = 3;
    settings.initial_dictionary_pos = 1;
    file->io.write(util::pack::lzss_decompress_bitwise(
        bit_reader, entry.size, settings));

    return file;
}

struct Pbg3Archive::Priv final
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
    io::BufferedIO buf_io;
    buf_io.write_from_io(arc_file.io, arc_file.io.size());
    buf_io.seek(magic.size());

    auto header = read_header(buf_io);
    auto table = read_table(buf_io, *header);

    for (auto &entry : table)
        file_saver.save(read_file(buf_io, *entry));
}

static auto dummy = fmt::Registry::add<Pbg3Archive>("th/pbg3");
