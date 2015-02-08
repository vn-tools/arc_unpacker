// NSA archive
//
// Company:   -
// Engine:    Nscripter
// Extension: .nsa
//
// Known games:
// - Tsukihime
// - Umineko no naku koro ni

#include "bit_reader.h"
#include "formats/arc/nsa_archive.h"

namespace
{
    typedef enum
    {
        COMPRESSION_NONE = 0,
        COMPRESSION_SPB = 1,
        COMPRESSION_LZSS = 2,
    } CompressionType;

    typedef struct
    {
        std::string name;
        CompressionType compression_type;
        size_t offset;
        size_t size_compressed;
        size_t size_original;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    Table read_table(IO &arc_io)
    {
        Table table;
        size_t file_count = arc_io.read_u16_be();
        size_t offset_to_files = arc_io.read_u32_be();
        if (offset_to_files > arc_io.size())
            throw std::runtime_error("Bad offset to files");

        for (size_t i = 0; i < file_count; i ++)
        {
            std::unique_ptr<TableEntry> entry(new TableEntry);
            entry->name = arc_io.read_until_zero();
            entry->compression_type
                = static_cast<CompressionType>(arc_io.read_u8());
            entry->offset = arc_io.read_u32_be();
            entry->size_compressed = arc_io.read_u32_be();
            entry->size_original = arc_io.read_u32_be();

            entry->offset += offset_to_files;

            if (entry->offset + entry->size_compressed > arc_io.size())
                throw std::runtime_error("Bad offset to file");

            table.push_back(std::move(entry));
        }
        return table;
    }

    void decompress_lzss(std::string &data, size_t size_original)
    {
        BufferedIO io(data);
        BitReader bit_reader(io);

        const size_t position_bits = 8;
        const size_t length_bits = 4;
        const size_t min_match_length = 2;
        const size_t initial_dictionary_pos = 239;
        const bool reuse_compressed = true;

        std::string output;
        output.reserve(size_original);
        size_t dictionary_size = 1 << position_bits;
        size_t dictionary_pos = initial_dictionary_pos;
        std::unique_ptr<unsigned char> dictionary(
            new unsigned char[dictionary_size]);

        unsigned char *dictionary_ptr = dictionary.get();

        size_t written = 0;
        while (written < size_original)
        {
            bool flag = bit_reader.get();

            if (flag)
            {
                unsigned char byte = bit_reader.get(8);
                output.push_back(byte);
                written ++;
                dictionary_ptr[dictionary_pos] = byte;
                dictionary_pos ++;
                dictionary_pos &= (dictionary_size - 1);
            }
            else
            {
                unsigned int pos = bit_reader.get(position_bits);
                unsigned int length = bit_reader.get(length_bits);
                length += min_match_length;
                for (size_t i = 0; i < length && written < size_original; i ++)
                {
                    unsigned char byte = dictionary_ptr[pos];
                    pos += 1;
                    pos &= (dictionary_size - 1);

                    if (reuse_compressed)
                    {
                        dictionary_ptr[dictionary_pos] = byte;
                        dictionary_pos ++;
                        dictionary_pos &= (dictionary_size - 1);
                    }
                    output.push_back(byte);
                    written ++;
                }
            }
        }

        data = output;
    }

    std::unique_ptr<VirtualFile> read_file(
        IO &arc_io, const TableEntry &table_entry)
    {
        std::unique_ptr<VirtualFile> file(new VirtualFile);

        file->name = table_entry.name;
        arc_io.seek(table_entry.offset);
        std::string data = arc_io.read(table_entry.size_compressed);

        switch (table_entry.compression_type)
        {
            case COMPRESSION_NONE:
                break;

            case COMPRESSION_LZSS:
                decompress_lzss(data, table_entry.size_original);
                break;

            case COMPRESSION_SPB:
                throw std::runtime_error("Not supported yet");
                break;
        }

        file->io.write(data);
        return file;
    }
}

struct NsaArchive::Internals
{
    // SPB converter will go here
};

NsaArchive::NsaArchive() : internals(new Internals)
{
}

NsaArchive::~NsaArchive()
{
}

void NsaArchive::unpack_internal(IO &arc_io, OutputFiles &output_files) const
{
    Table table = read_table(arc_io);
    for (size_t i = 0; i < table.size(); i ++)
    {
        output_files.save([&]() -> std::unique_ptr<VirtualFile>
        {
            return read_file(arc_io, *table[i]);
        });
    }
}
