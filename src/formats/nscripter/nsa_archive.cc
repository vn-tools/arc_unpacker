// NSA archive
//
// Company:   -
// Engine:    NScripter
// Extension: .nsa
//
// Known games:
// - Tsukihime
// - Umineko no naku koro ni

#include "bit_reader.h"
#include "buffered_io.h"
#include "formats/nscripter/nsa_archive.h"
#include "formats/nscripter/spb_converter.h"
#include "util/lzss.h"
using namespace Formats::NScripter;

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

    std::unique_ptr<File> read_file(
        IO &arc_io, const TableEntry &table_entry, SpbConverter &spb_converter)
    {
        std::unique_ptr<File> file(new File);

        file->name = table_entry.name;
        arc_io.seek(table_entry.offset);
        std::string data = arc_io.read(table_entry.size_compressed);

        switch (table_entry.compression_type)
        {
            case COMPRESSION_NONE:
                file->io.write(data);
                break;

            case COMPRESSION_LZSS:
            {
                BufferedIO data_io(data);
                BitReader bit_reader(data_io);

                LzssSettings settings;
                settings.position_bits = 8;
                settings.length_bits = 4;
                settings.min_match_length = 2;
                settings.initial_dictionary_pos = 239;
                settings.reuse_compressed = true;
                file->io.write(lzss_decompress(
                    bit_reader,
                    table_entry.size_original,
                    settings));
                break;
            }

            case COMPRESSION_SPB:
                file->io.write(data);
                file = spb_converter.decode(*file);
                break;
        }

        return file;
    }
}

struct NsaArchive::Internals
{
    SpbConverter spb_converter;
};

NsaArchive::NsaArchive() : internals(new Internals)
{
}

NsaArchive::~NsaArchive()
{
}

void NsaArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    Table table = read_table(arc_file.io);
    for (auto &table_entry : table)
    {
        file_saver.save(
            read_file(arc_file.io, *table_entry, internals->spb_converter));
    }
}
