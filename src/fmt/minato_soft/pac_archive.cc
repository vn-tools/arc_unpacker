// PAC archive
//
// Company:   Minato Soft
// Engine:    -
// Extension: .pac
//
// Known games:
// - Maji de Watashi ni Koishinasai!

#include "fmt/minato_soft/pac_archive.h"
#include "io/bit_reader.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::minato_soft;

namespace
{
    struct TableEntry
    {
        std::string name;
        size_t offset;
        size_t size_original;
        size_t size_compressed;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static const bstr magic = "PAC\x00"_b;

static int init_huffman(io::BitReader &bit_reader, u16 nodes[2][512], int &pos)
{
    if (bit_reader.get(1))
    {
        int old_pos = pos;
        pos++;
        if (old_pos < 511)
        {
            nodes[0][old_pos] = init_huffman(bit_reader, nodes, pos);
            nodes[1][old_pos] = init_huffman(bit_reader, nodes, pos);
            return old_pos;
        }
        return -1;
    }
    return bit_reader.get(8);
}

static bstr decompress_table(const bstr &input, size_t output_size)
{
    bstr output;
    output.resize(output_size);
    u8 *output_ptr = output.get<u8>();
    u8 *output_end = output_ptr + output_size;
    io::BitReader bit_reader(input);

    u16 nodes[2][512];
    int pos = 256;
    int initial_pos = init_huffman(bit_reader, nodes, pos);

    while (output_ptr < output_end)
    {
        unsigned int pos = initial_pos;
        while (pos >= 256)
            pos = nodes[bit_reader.get(1)][pos];

        *output_ptr++ = pos;
    }
    return output;
}

static Table read_table(io::IO &arc_io, size_t file_count)
{
    arc_io.seek(arc_io.size() - 4);
    size_t compressed_size = arc_io.read_u32_le();
    size_t uncompressed_size = file_count * 76;

    arc_io.seek(arc_io.size() - 4 - compressed_size);
    bstr compressed = arc_io.read(compressed_size);
    for (auto i : util::range(compressed.size()))
        compressed.get<u8>()[i] ^= 0xFF;

    io::BufferedIO table_io(decompress_table(compressed, uncompressed_size));
    table_io.seek(0);

    Table table;
    for (auto i : util::range(file_count))
    {
        size_t pos = table_io.tell();
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = util::sjis_to_utf8(table_io.read_to_zero()).str();
        table_io.seek(pos + 0x40);
        entry->offset = table_io.read_u32_le();
        entry->size_original = table_io.read_u32_le();
        entry->size_compressed = table_io.read_u32_le();
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    arc_io.seek(entry.offset);
    if (entry.size_original == entry.size_compressed)
    {
        file->io.write_from_io(arc_io, entry.size_original);
    }
    else
    {
        file->io.write(
            util::pack::zlib_inflate(
                arc_io.read(entry.size_compressed)));
    }
    file->name = entry.name;
    return file;
}

bool PacArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void PacArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());

    size_t file_count = arc_file.io.read_u32_le();
    auto table = read_table(arc_file.io, file_count);

    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<PacArchive>("minato/pac");
