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
#include "util/zlib.h"

using namespace au;
using namespace au::fmt::minato_soft;

namespace
{
    typedef struct
    {
        std::string name;
        size_t offset;
        size_t size_original;
        size_t size_compressed;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;
}

static const std::string magic = "PAC\x00"_s;

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

static void decompress_table(
    const char *input, int input_size, char *output, int output_size)
{
    char *output_guardian = output + output_size;
    std::unique_ptr<io::BitReader> bit_reader(
        new io::BitReader(input, input_size));

    u16 nodes[2][512];
    int pos = 256;
    int initial_pos = init_huffman(*bit_reader, nodes, pos);

    while (output < output_guardian)
    {
        unsigned int pos = initial_pos;
        while (pos >= 256)
            pos = nodes[bit_reader->get(1)][pos];

        *output++ = pos;
    }
}

static Table read_table(io::IO &arc_io, size_t file_count)
{
    arc_io.seek(arc_io.size() - 4);
    size_t compressed_size = arc_io.read_u32_le();
    size_t uncompressed_size = file_count * 76;

    std::unique_ptr<char[]> compressed(new char[compressed_size]);
    arc_io.seek(arc_io.size() - 4 - compressed_size);
    arc_io.read(compressed.get(), compressed_size);
    for (size_t i = 0; i < compressed_size; i++)
        compressed.get()[i] ^= 0xFF;

    std::unique_ptr<char[]> uncompressed(new char[uncompressed_size]);
    for (size_t i = 0; i < uncompressed_size; i++)
        uncompressed.get()[i] = 0;

    decompress_table(
        compressed.get(),
        compressed_size,
        uncompressed.get(),
        uncompressed_size);

    io::BufferedIO table_io(uncompressed.get(), uncompressed_size);
    table_io.seek(0);

    Table table;
    for (size_t i = 0; i < file_count; i++)
    {
        size_t pos = table_io.tell();
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = table_io.read_until_zero();
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
        std::string data = arc_io.read(entry.size_compressed);
        data = util::zlib_inflate(data);
        file->io.write(data);
    }
    file->name = util::sjis_to_utf8(entry.name);
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
