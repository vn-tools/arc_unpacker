// THA1 archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .dat
//
// Known games:
// - Touhou 10 - Mountain of Faith

#include "buffered_io.h"
#include "formats/arc/pbgz_archive/crypt.h"
#include "formats/arc/tha1_archive.h"
#include "formats/gfx/anm_archive.h"
#include "string/lzss.h"

#include <iostream>
using namespace std;
namespace
{
    const std::string magic("THA1", 4);

    typedef struct
    {
        size_t file_count;
        size_t table_offset;
        size_t table_size_original;
        size_t table_size_compressed;
    } Header;

    typedef struct
    {
        std::string name;
        size_t offset;
        size_t size_compressed;
        size_t size_original;
        uint8_t decryptor_id;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    std::vector<std::vector<DecryptorContext>> decryptors
    {
        {
            { 0x1b, 0x37,  0x40, 0x2800 },
            { 0x51, 0xe9,  0x40, 0x3000 },
            { 0xc1, 0x51,  0x80, 0x3200 },
            { 0x03, 0x19, 0x400, 0x7800 },
            { 0xab, 0xcd, 0x200, 0x2800 },
            { 0x12, 0x34,  0x80, 0x3200 },
            { 0x35, 0x97,  0x80, 0x2800 },
            { 0x99, 0x37, 0x400, 0x2000 },
        },
        {
            { 0x1b, 0x73,  0x40, 0x3800 },
            { 0x51, 0x9e,  0x40, 0x4000 },
            { 0xc1, 0x15, 0x400, 0x2c00 },
            { 0x03, 0x91,  0x80, 0x6400 },
            { 0xab, 0xdc,  0x80, 0x6e00 },
            { 0x12, 0x43, 0x200, 0x3c00 },
            { 0x35, 0x79, 0x400, 0x3c00 },
            { 0x99, 0x7d,  0x80, 0x2800 },
        },
        {
            { 0x1b, 0x73, 0x0100, 0x3800 },
            { 0x12, 0x43, 0x0200, 0x3e00 },
            { 0x35, 0x79, 0x0400, 0x3c00 },
            { 0x03, 0x91, 0x0080, 0x6400 },
            { 0xab, 0xdc, 0x0080, 0x6e00 },
            { 0x51, 0x9e, 0x0100, 0x4000 },
            { 0xc1, 0x15, 0x0400, 0x2c00 },
            { 0x99, 0x7d, 0x0080, 0x4400 },
        },
    };

    std::string lzss_decompress(
        IO &io,
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
        buffered_io.write_from_io(io, size_compressed);
        buffered_io.seek(0);
        BitReader bit_reader(buffered_io);
        return ::lzss_decompress(bit_reader, size_original, settings);
    }

    std::unique_ptr<Header> read_header(IO &arc_io)
    {
        std::unique_ptr<Header> header(new Header);
        BufferedIO header_io;
        decrypt(arc_io, 16, header_io, { 0x1b, 0x37, 0x10, 0x400 });
        if (header_io.read(magic.size()) != magic)
            throw std::runtime_error("Not a THA1 archive");
        header->table_size_original = header_io.read_u32_le() - 123456789;
        header->table_size_compressed = header_io.read_u32_le() - 987654321;
        header->file_count = header_io.read_u32_le() - 135792468;
        header->table_offset = arc_io.size() - header->table_size_compressed;
        return header;
    }

    std::unique_ptr<BufferedIO> read_raw_table(IO &arc_io, const Header &header)
    {
        BufferedIO decrypted_io;
        arc_io.seek(header.table_offset);
        decrypt(
            arc_io,
            header.table_size_compressed,
            decrypted_io,
            { 0x3e, 0x9b, 0x80, header.table_size_compressed });

        return std::unique_ptr<BufferedIO>(
            new BufferedIO(
                lzss_decompress(
                    decrypted_io,
                    header.table_size_compressed,
                    header.table_size_original)));
    }

    Table read_table(IO &arc_io, const Header &header)
    {
        Table table;
        auto table_io = read_raw_table(arc_io, header);

        for (size_t i = 0; i < header.file_count; i ++)
        {
            std::unique_ptr<TableEntry> table_entry(new TableEntry);

            table_entry->name = table_io->read_until_zero();
            table_io->skip(3 - table_entry->name.length() % 4);

            table_entry->decryptor_id = 0;
            for (size_t j = 0; j < table_entry->name.length(); j ++)
                table_entry->decryptor_id += table_entry->name[j];
            table_entry->decryptor_id %= 8;

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

        for (auto &table_entry : table)
        {
            cout << table_entry->name << ": " << (int) table_entry->decryptor_id << " " << table_entry->size_compressed << endl;
        }

        return table;
    }

    std::unique_ptr<File> read_file(
        IO &arc_io, const TableEntry &table_entry, size_t encryption_version)
    {
        std::unique_ptr<File> file(new File);
        file->name = table_entry.name;

        arc_io.seek(table_entry.offset);
        BufferedIO decrypted_io;
        decrypt(
            arc_io,
            table_entry.size_compressed,
            decrypted_io,
            decryptors[encryption_version][table_entry.decryptor_id]);

        if (table_entry.size_compressed == table_entry.size_original)
        {
            file->io.write_from_io(decrypted_io, table_entry.size_original);
        }
        else
        {
            file->io.write(lzss_decompress(
                decrypted_io,
                table_entry.size_compressed,
                table_entry.size_original));
        }

        return file;
    }

    size_t detect_encryption_version(IO &arc_io, const Table &table)
    {
        const std::string riff_magic("RIFF", 4);
        for (auto &table_entry : table)
        {
            if (table_entry->name.find(".wav") == std::string::npos)
                continue;
            for (size_t version = 0; version < decryptors.size(); version ++)
            {
                auto file = read_file(arc_io, *table_entry, version);
                file->io.seek(0);
                if (file->io.read(riff_magic.size()) == riff_magic)
                    return version;
            }
        }
        throw std::runtime_error("No means to detect the encryption version");
    }
}

struct Tha1Archive::Internals
{
    AnmArchive anm_archive;
};

Tha1Archive::Tha1Archive() : internals(new Internals())
{
}

Tha1Archive::~Tha1Archive()
{
}

void Tha1Archive::add_cli_help(ArgParser &arg_parser) const
{
    internals->anm_archive.add_cli_help(arg_parser);
}

void Tha1Archive::parse_cli_options(ArgParser &arg_parser)
{
    internals->anm_archive.parse_cli_options(arg_parser);
}

void Tha1Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto header = read_header(arc_file.io);
    auto table = read_table(arc_file.io, *header);
    auto encryption_version = detect_encryption_version(arc_file.io, table);

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
