// PBGZ archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .dat
//
// Known games:
// - Touhou 08 - Imperishable Night
// - Touhou 09 - Phantasmagoria of Flower View

#include <cstring>
#include <map>
#include "buffered_io.h"
#include "formats/arc/pbgz_archive.h"
#include "formats/arc/pbgz_archive/crypt.h"
#include "formats/gfx/anm_archive.h"
#include "string/lzss.h"

namespace
{
    const std::string magic("PBGZ", 4);

    typedef struct
    {
        size_t file_count;
        size_t table_offset;
        size_t table_size;
    } Header;

    typedef struct
    {
        size_t size_compressed;
        size_t size_original;
        size_t offset;
        std::string name;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    std::vector<std::map<uint8_t, DecryptorContext>> decryptors
    {
        {
            { 0x2a, { 0x99, 0x37,  0x400, 0x1000 } },
            { 0x2d, { 0x35, 0x97,   0x80, 0x2800 } },
            { 0x41, { 0xc1, 0x51,  0x400,  0x400 } },
            { 0x45, { 0xab, 0xcd,  0x200, 0x1000 } },
            { 0x4a, { 0x03, 0x19,  0x400,  0x400 } },
            { 0x4d, { 0x1b, 0x37,   0x40, 0x2800 } },
            { 0x54, { 0x51, 0xe9,   0x40, 0x3000 } },
            { 0x57, { 0x12, 0x34,  0x400,  0x400 } },
        },
        {
            { 0x2a, { 0x99, 0x37,  0x400, 0x1000 } },
            { 0x2d, { 0x35, 0x97,   0x80, 0x2800 } },
            { 0x41, { 0xc1, 0x51, 0x1400, 0x2000 } },
            { 0x45, { 0xab, 0xcd,  0x200, 0x1000 } },
            { 0x4a, { 0x03, 0x19, 0x1400, 0x7800 } },
            { 0x4d, { 0x1b, 0x37,   0x40, 0x2000 } },
            { 0x54, { 0x51, 0xe9,   0x40, 0x3000 } },
            { 0x57, { 0x12, 0x34,  0x400, 0x2800 } },
        },
    };

    std::string lzss_decompress(
        IO &arc_io,
        size_t size_compressed,
        size_t size_original)
    {
        LzssSettings settings;
        settings.position_bits = 13;
        settings.length_bits = 4;
        settings.min_match_length = 3;
        settings.initial_dictionary_pos = 1;
        settings.reuse_compressed = true;

        BufferedIO buffered_io;
        buffered_io.write_from_io(arc_io, size_compressed);
        buffered_io.seek(0);
        BitReader bit_reader(buffered_io);
        return ::lzss_decompress(bit_reader, size_original, settings);
    }

    std::unique_ptr<Header> read_header(IO &arc_io)
    {
        std::unique_ptr<Header> header(new Header);
        BufferedIO header_io;
        decrypt(arc_io, 12, header_io, { 0x1b, 0x37, 0x0c, 0x400 });
        header->file_count = header_io.read_u32_le() - 123456;
        header->table_offset = header_io.read_u32_le() - 345678;
        header->table_size = header_io.read_u32_le() - 567891;
        return header;
    }

    std::unique_ptr<BufferedIO> read_raw_table(IO &arc_io, const Header &header)
    {
        size_t size_compressed = arc_io.size() - header.table_offset;
        size_t size_original = header.table_size;

        BufferedIO table_io;
        arc_io.seek(header.table_offset);
        decrypt(arc_io, size_compressed, table_io, { 0x3e, 0x9b, 0x80, 0x400 });

        return std::unique_ptr<BufferedIO>(
            new BufferedIO(
                lzss_decompress(table_io, size_compressed, size_original)));
    }

    Table read_table(IO &arc_io, const Header &header)
    {
        Table table;
        auto table_io = read_raw_table(arc_io, header);

        for (size_t i = 0; i < header.file_count; i ++)
        {
            std::unique_ptr<TableEntry> table_entry(new TableEntry);
            table_entry->name = table_io->read_until_zero();
            table_entry->offset = table_io->read_u32_le();
            table_entry->size_original = table_io->read_u32_le();
            table_io->skip(4);
            table.push_back(std::move(table_entry));
        }

        for (size_t i = 0; i < table.size() - 1; i ++)
            table[i]->size_compressed = table[i + 1]->offset - table[i]->offset;

        if (table.size() > 0)
        {
            table[table.size() - 1]->size_compressed
                = header.table_offset - table[table.size() - 1]->offset;
        }

        return table;
    }

    std::unique_ptr<File> read_file(
        IO &arc_io, const TableEntry &table_entry, size_t encryption_version)
    {
        std::unique_ptr<File> file(new File);
        file->name = table_entry.name;

        arc_io.seek(table_entry.offset);
        BufferedIO uncompressed_io(
            lzss_decompress(
                arc_io,
                table_entry.size_compressed,
                table_entry.size_original));

        const std::string crypt_magic("edz", 3);
        if (uncompressed_io.read(crypt_magic.size()) != crypt_magic)
            throw std::runtime_error("Unknown encryption");

        decrypt(
            uncompressed_io,
            table_entry.size_original,
            file->io,
            decryptors[encryption_version][uncompressed_io.read_u8()]);

        return file;
    }

    size_t detect_encryption_version(IO &arc_io, const Table &table)
    {
        const std::string jpeg_magic("\xff\xd8\xff\xe0", 4);
        for (auto &table_entry : table)
        {
            if (table_entry->name.find(".jpg") == std::string::npos)
                continue;
            for (size_t version = 0; version < decryptors.size(); version ++)
            {
                auto file = read_file(arc_io, *table_entry, version);
                file->io.seek(0);
                if (file->io.read(jpeg_magic.size()) == jpeg_magic)
                    return version;
            }
        }
        throw std::runtime_error("No means to detect the encryption version");
    }
}

struct PbgzArchive::Internals
{
    AnmArchive anm_archive;
};

PbgzArchive::PbgzArchive() : internals(new Internals())
{
}

PbgzArchive::~PbgzArchive()
{
}

void PbgzArchive::add_cli_help(ArgParser &arg_parser) const
{
    internals->anm_archive.add_cli_help(arg_parser);
}

void PbgzArchive::parse_cli_options(ArgParser &arg_parser)
{
    internals->anm_archive.parse_cli_options(arg_parser);
}

void PbgzArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    if (arc_file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a PBGZ archive");

    auto header = read_header(arc_file.io);
    auto table = read_table(arc_file.io, *header);
    int encryption_version = detect_encryption_version(arc_file.io, table);

    for (auto &table_entry : table)
    {
        auto file = read_file(arc_file.io, *table_entry, encryption_version);
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
