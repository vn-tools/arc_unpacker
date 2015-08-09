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
#include <iostream>
#include <map>
#include "fmt/touhou/anm_archive.h"
#include "fmt/touhou/crypt.h"
#include "fmt/touhou/pbgz_archive.h"
#include "io/buffered_io.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

namespace
{
    struct Header
    {
        size_t file_count;
        size_t table_offset;
        size_t table_size;
    };

    struct TableEntry
    {
        size_t size_compressed;
        size_t size_original;
        size_t offset;
        std::string name;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static const bstr crypt_magic = "edz"_b;
static const bstr jpeg_magic = "\xFF\xD8\xFF\xE0"_b;
static const bstr magic = "PBGZ"_b;

static std::vector<std::map<u8, DecryptorContext>> decryptors
{
    {
        { 0x2A, { 0x99, 0x37,  0x400, 0x1000 } },
        { 0x2D, { 0x35, 0x97,   0x80, 0x2800 } },
        { 0x41, { 0xC1, 0x51,  0x400,  0x400 } },
        { 0x45, { 0xAB, 0xCD,  0x200, 0x1000 } },
        { 0x4A, { 0x03, 0x19,  0x400,  0x400 } },
        { 0x4D, { 0x1B, 0x37,   0x40, 0x2800 } },
        { 0x54, { 0x51, 0xE9,   0x40, 0x3000 } },
        { 0x57, { 0x12, 0x34,  0x400,  0x400 } },
    },
    {
        { 0x2A, { 0x99, 0x37,  0x400, 0x1000 } },
        { 0x2D, { 0x35, 0x97,   0x80, 0x2800 } },
        { 0x41, { 0xC1, 0x51, 0x1400, 0x2000 } },
        { 0x45, { 0xAB, 0xCD,  0x200, 0x1000 } },
        { 0x4A, { 0x03, 0x19, 0x1400, 0x7800 } },
        { 0x4D, { 0x1B, 0x37,   0x40, 0x2000 } },
        { 0x54, { 0x51, 0xE9,   0x40, 0x3000 } },
        { 0x57, { 0x12, 0x34,  0x400, 0x2800 } },
    },
};

static bstr decompress(const bstr &input, size_t size_original)
{
    util::pack::LzssSettings settings;
    settings.position_bits = 13;
    settings.length_bits = 4;
    settings.min_match_length = 3;
    settings.initial_dictionary_pos = 1;
    settings.reuse_compressed = true;
    return util::pack::lzss_decompress(input, size_original, settings);
}

static std::unique_ptr<Header> read_header(io::IO &arc_io)
{
    std::unique_ptr<Header> header(new Header);
    io::BufferedIO header_io(
        decrypt(arc_io.read(12), { 0x1B, 0x37, 0x0C, 0x400 }));
    header->file_count = header_io.read_u32_le() - 123456;
    header->table_offset = header_io.read_u32_le() - 345678;
    header->table_size = header_io.read_u32_le() - 567891;
    return header;
}

static Table read_table(io::IO &arc_io, const Header &header)
{
    arc_io.seek(header.table_offset);
    io::BufferedIO table_io(
        decompress(
            decrypt(arc_io.read_until_end(), { 0x3E, 0x9B, 0x80, 0x400 }),
            header.table_size));

    Table table;
    for (auto i : util::range(header.file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = table_io.read_until_zero().str();
        entry->offset = table_io.read_u32_le();
        entry->size_original = table_io.read_u32_le();
        table_io.skip(4);
        table.push_back(std::move(entry));
    }

    for (auto i : util::range(table.size() - 1))
        table[i]->size_compressed = table[i + 1]->offset - table[i]->offset;

    if (table.size() > 0)
    {
        table[table.size() - 1]->size_compressed
            = header.table_offset - table[table.size() - 1]->offset;
    }

    return table;
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io, const TableEntry &entry, size_t encryption_version)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;

    arc_io.seek(entry.offset);
    io::BufferedIO uncompressed_io(
        decompress(
            arc_io.read(entry.size_compressed),
            entry.size_original));

    if (uncompressed_io.read(crypt_magic.size()) != crypt_magic)
        throw std::runtime_error("Unknown encryption");

    file->io.write(
        decrypt(
            uncompressed_io.read_until_end(),
            decryptors[encryption_version][uncompressed_io.read_u8()]));

    return file;
}

static size_t detect_encryption_version(io::IO &arc_io, const Table &table)
{
    for (auto &entry : table)
    {
        if (entry->name.find(".jpg") == std::string::npos)
            continue;
        for (auto version : util::range(decryptors.size()))
        {
            auto file = read_file(arc_io, *entry, version);
            file->io.seek(0);
            if (file->io.read(jpeg_magic.size()) == jpeg_magic)
                return version;
        }
    }
    throw std::runtime_error("No means to detect the encryption version");
}

struct PbgzArchive::Priv
{
    AnmArchive anm_archive;
};

PbgzArchive::PbgzArchive() : p(new Priv)
{
    add_transformer(&p->anm_archive);
}

PbgzArchive::~PbgzArchive()
{
}

bool PbgzArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void PbgzArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());
    auto header = read_header(arc_file.io);
    auto table = read_table(arc_file.io, *header);
    int encryption_version = detect_encryption_version(arc_file.io, table);

    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry, encryption_version));
}
