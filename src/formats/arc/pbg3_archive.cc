// PBG3 archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .dat
//
// Known games:
// - Touhou 06 - The Embodiment of Scarlet Devil

#include "bit_reader.h"
#include "buffered_io.h"
#include "formats/arc/pbg3_archive.h"

#include <iostream>
namespace
{
    const std::string magic("PBG3", 4);

    typedef struct
    {
        size_t file_count;
        size_t table_offset;
    } Header;

    typedef struct
    {
        uint32_t checksum;
        size_t size;
        size_t offset;
        std::string name;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    unsigned int read_integer(BitReader &bit_reader)
    {
        size_t integer_size = bit_reader.get(2) + 1;
        return bit_reader.get(integer_size << 3);
    }

    std::unique_ptr<Header> read_header(IO &arc_io)
    {
        std::unique_ptr<Header> header(new Header);
        BitReader bit_reader(arc_io);
        header->file_count = read_integer(bit_reader);
        header->table_offset = read_integer(bit_reader);
        return header;
    }

    Table read_table(IO &arc_io, Header &header)
    {
        Table table;
        arc_io.seek(header.table_offset);
        BitReader bit_reader(arc_io);
        for (size_t i = 0; i < header.file_count; i ++)
        {
            std::unique_ptr<TableEntry> table_entry(new TableEntry);
            read_integer(bit_reader);
            read_integer(bit_reader);
            table_entry->checksum = read_integer(bit_reader);
            table_entry->offset = read_integer(bit_reader);
            table_entry->size = read_integer(bit_reader);
            for (size_t i = 0; i < 256; i ++)
            {
                char c = static_cast<char>(bit_reader.get(8));
                if (c == 0)
                    break;
                table_entry->name.push_back(c);
            }
            table.push_back(std::move(table_entry));
        }
        return table;
    }

    // TODO: almost the same code is in NSA
    std::string decompress_lzss(BitReader &bit_reader, size_t size_original)
    {
        const size_t position_bits = 13;
        const size_t length_bits = 4;
        const size_t min_match_length = 3;
        const size_t initial_dictionary_pos = 1;
        const bool reuse_compressed = true;

        std::string output;
        size_t dictionary_size = 1 << position_bits;
        size_t dictionary_pos = initial_dictionary_pos;
        std::unique_ptr<unsigned char> dictionary(
            new unsigned char[dictionary_size]);

        unsigned char *dictionary_ptr = dictionary.get();

        while (output.size() < size_original)
        {
            if (bit_reader.get(1) > 0)
            {
                unsigned char byte = bit_reader.get(8);
                output.push_back(byte);
                dictionary_ptr[dictionary_pos] = byte;
                dictionary_pos ++;
                dictionary_pos %= dictionary_size;
            }
            else
            {
                unsigned int pos = bit_reader.get(position_bits);
                unsigned int length = bit_reader.get(length_bits);
                length += min_match_length;
                for (size_t i = 0; i < length; i ++)
                {
                    unsigned char byte = dictionary_ptr[pos];
                    pos += 1;
                    pos %= dictionary_size;

                    if (reuse_compressed)
                    {
                        dictionary_ptr[dictionary_pos] = byte;
                        dictionary_pos ++;
                        dictionary_pos %= dictionary_size;
                    }
                    output.push_back(byte);
                }
            }
        }

        return output;
    }

    std::unique_ptr<File> read_file(IO &arc_io, const TableEntry &table_entry)
    {
        std::unique_ptr<File> file(new File);
        file->name = table_entry.name;

        arc_io.seek(table_entry.offset);
        BitReader bit_reader(arc_io);
        file->io.write(decompress_lzss(bit_reader, table_entry.size));

        return file;
    }
}

void Pbg3Archive::unpack_internal(File &file, FileSaver &file_saver) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a PBG3 archive");

    // work much faster when the whole archive resides in memory
    file.io.seek(0);
    BufferedIO buf_io;
    buf_io.write_from_io(file.io, file.io.size());
    buf_io.seek(magic.size());

    auto header = read_header(buf_io);
    auto table = read_table(buf_io, *header);

    for (auto &table_entry : table)
        file_saver.save(read_file(buf_io, *table_entry));
}
