// PBG4 archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .dat
//
// Known games:
// - Touhou 07 - Perfect Cherry Blossom

#include "buffered_io.h"
#include "formats/arc/pbg4_archive.h"
#include "formats/gfx/anm_archive.h"
#include "string/lzss.h"

namespace
{
    const std::string magic("PBG4", 4);

    typedef struct
    {
        size_t file_count;
        size_t table_offset;
        size_t table_size;
    } Header;

    typedef struct
    {
        size_t size;
        size_t offset;
        std::string name;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    std::unique_ptr<Header> read_header(IO &arc_io)
    {
        std::unique_ptr<Header> header(new Header);
        header->file_count = arc_io.read_u32_le();
        header->table_offset = arc_io.read_u32_le();
        header->table_size = arc_io.read_u32_le();
        return header;
    }

    std::string lzss_decompress(IO &arc_io, size_t size_original)
    {
        LzssSettings settings;
        settings.position_bits = 13;
        settings.length_bits = 4;
        settings.min_match_length = 3;
        settings.initial_dictionary_pos = 1;
        settings.reuse_compressed = true;
        BitReader bit_reader(arc_io);
        return ::lzss_decompress(bit_reader, size_original, settings);
    }

    Table read_table(IO &arc_io, Header &header)
    {
        Table table;
        arc_io.seek(header.table_offset);

        BufferedIO table_io(lzss_decompress(arc_io, header.table_size));
        for (size_t i = 0; i < header.file_count; i ++)
        {
            std::unique_ptr<TableEntry> table_entry(new TableEntry);
            while (true)
            {
                char c = static_cast<char>(table_io.read_u8());
                if (c == 0)
                    break;
                table_entry->name.push_back(c);
            }

            table_entry->offset = table_io.read_u32_le();
            table_entry->size = table_io.read_u32_le();
            table_io.skip(4);

            table.push_back(std::move(table_entry));
        }
        return table;
    }

    std::unique_ptr<File> read_file(IO &arc_io, const TableEntry &table_entry)
    {
        std::unique_ptr<File> file(new File);
        arc_io.seek(table_entry.offset);
        file->io.write(lzss_decompress(arc_io, table_entry.size));
        file->name = table_entry.name;
        return file;
    }
}

struct Pbg4Archive::Internals
{
    AnmArchive anm_archive;
};

Pbg4Archive::Pbg4Archive() : internals(new Internals())
{
}

Pbg4Archive::~Pbg4Archive()
{
}

void Pbg4Archive::add_cli_help(ArgParser &arg_parser) const
{
    internals->anm_archive.add_cli_help(arg_parser);
}

void Pbg4Archive::parse_cli_options(ArgParser &arg_parser)
{
    internals->anm_archive.parse_cli_options(arg_parser);
}

void Pbg4Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    if (arc_file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a PBG4 archive");

    // works much faster when the whole archive resides in memory
    arc_file.io.seek(0);
    BufferedIO buf_io;
    buf_io.write_from_io(arc_file.io, arc_file.io.size());
    buf_io.seek(magic.size());

    auto header = read_header(buf_io);
    auto table = read_table(buf_io, *header);

    for (auto &table_entry : table)
    {
        auto file = read_file(buf_io, *table_entry);
        try
        {
            internals->anm_archive.unpack(*file, file_saver);
        }
        catch (...)
        {
            file_saver.save(std::move(file));
        }
    }
}
