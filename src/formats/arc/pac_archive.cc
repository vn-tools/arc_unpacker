// PAC archive
//
// Company:   Minato Soft
// Engine:    -
// Extension: .pac
//
// Known games:
// - Maji de Watashi ni Koishinasai!

#include "buffered_io.h"
#include "bit_reader.h"
#include "formats/arc/pac_archive.h"
#include "string_ex.h"

namespace
{
    const std::string magic("PAC\x00", 4);

    typedef struct
    {
        std::string name;
        size_t offset;
        size_t size_original;
        size_t size_compressed;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    int init_huffman(
        BitReader &bit_reader, uint16_t nodes[2][512], int &pos)
    {
        if (bit_reader.get(1))
        {
            int old_pos = pos;
            pos ++;
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

    void decompress_table(
        const char *input, int input_size, char *output, int output_size)
    {
        char *output_guardian = output + output_size;
        std::unique_ptr<BitReader> bit_reader(new BitReader(input, input_size));

        uint16_t nodes[2][512];
        int pos = 256;
        int initial_pos = init_huffman(*bit_reader, nodes, pos);

        while (output < output_guardian)
        {
            unsigned int pos = initial_pos;
            while (pos >= 256)
            {
                if (bit_reader->get(1))
                    pos = nodes[1][pos];
                else
                    pos = nodes[0][pos];
            }

            *output ++ = pos;
        }
    }

    Table read_table(IO &arc_io, size_t file_count)
    {
        arc_io.seek(arc_io.size() - 4);
        size_t compressed_size = arc_io.read_u32_le();
        size_t uncompressed_size = file_count * 76;

        std::unique_ptr<char[]> compressed(new char[compressed_size]);
        arc_io.seek(arc_io.size() - 4 - compressed_size);
        arc_io.read(compressed.get(), compressed_size);
        for (size_t i = 0; i < compressed_size; i ++)
            compressed.get()[i] ^= 0xff;

        std::unique_ptr<char[]> uncompressed(new char[uncompressed_size]);
        for (size_t i = 0; i < uncompressed_size; i ++)
            uncompressed.get()[i] = 0;

        decompress_table(
            compressed.get(),
            compressed_size,
            uncompressed.get(),
            uncompressed_size);

        BufferedIO table_io(uncompressed.get(), uncompressed_size);
        table_io.seek(0);

        Table table;
        for (size_t i = 0; i < file_count; i ++)
        {
            size_t pos = table_io.tell();
            std::unique_ptr<TableEntry> table_entry(new TableEntry);
            table_entry->name = table_io.read_until_zero();
            table_io.seek(pos + 0x40);
            table_entry->offset = table_io.read_u32_le();
            table_entry->size_original = table_io.read_u32_le();
            table_entry->size_compressed = table_io.read_u32_le();
            table.push_back(std::move(table_entry));
        }
        return table;
    }

    std::unique_ptr<File> read_file(IO &arc_io, TableEntry &table_entry)
    {
        std::unique_ptr<File> file(new File);
        arc_io.seek(table_entry.offset);
        if (table_entry.size_original == table_entry.size_compressed)
        {
            file->io.write_from_io(arc_io, table_entry.size_original);
        }
        else
        {
            std::string data = arc_io.read(table_entry.size_compressed);
            data = zlib_inflate(data);
            file->io.write(data);
        }
        file->name = table_entry.name;
        return file;
    }
}

void PacArchive::unpack_internal(
    File &file, FileSaver &file_saver) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a PAC archive");

    size_t file_count = file.io.read_u32_le();
    auto table = read_table(file.io, file_count);

    for (auto &table_entry : table)
    {
        file_saver.save([&]()
        {
            return read_file(file.io, *table_entry);
        });
    }
}
