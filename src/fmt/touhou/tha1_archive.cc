// THA1 archive
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .dat
//
// Known games:
// - Touhou 09.5 - Shoot the Bullet
// - Touhou 10 - Mountain of Faith
// - Touhou 11 - Subterranean Animism
// - Touhou 12 - Undefined Fantastic Object
// - Touhou 12.5 - Double Spoiler
// - Touhou 12.8 - Fairy Wars
// - Touhou 13 - Ten Desires
// - Touhou 14 - Double Dealing Character
// - Touhou 14.3 - Impossible Spell Card
// - Touhou 15 - Legacy of Lunatic Kingdom (trial)

#include "fmt/touhou/anm_archive.h"
#include "fmt/touhou/crypt.h"
#include "fmt/touhou/tha1_archive.h"
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
        size_t table_size_original;
        size_t table_size_compressed;
    };

    struct TableEntry
    {
        std::string name;
        size_t offset;
        size_t size_compressed;
        size_t size_original;
        u8 decryptor_id;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static const std::string magic = "THA1"_s;

static std::vector<std::vector<DecryptorContext>> decryptors
{
    //TH9.5, TH10, TH11
    {
        { 0x1B, 0x37,  0x40, 0x2800 },
        { 0x51, 0xE9,  0x40, 0x3000 },
        { 0xC1, 0x51,  0x80, 0x3200 },
        { 0x03, 0x19, 0x400, 0x7800 },
        { 0xAB, 0xCD, 0x200, 0x2800 },
        { 0x12, 0x34,  0x80, 0x3200 },
        { 0x35, 0x97,  0x80, 0x2800 },
        { 0x99, 0x37, 0x400, 0x2000 },
    },

    //TH12, TH12.5, TH12.8
    {
        { 0x1B, 0x73,  0x40, 0x3800 },
        { 0x51, 0x9E,  0x40, 0x4000 },
        { 0xC1, 0x15, 0x400, 0x2C00 },
        { 0x03, 0x91,  0x80, 0x6400 },
        { 0xAB, 0xDC,  0x80, 0x6E00 },
        { 0x12, 0x43, 0x200, 0x3C00 },
        { 0x35, 0x79, 0x400, 0x3C00 },
        { 0x99, 0x7D,  0x80, 0x2800 },
    },

    //TH13
    {
        { 0x1B, 0x73, 0x0100, 0x3800 },
        { 0x12, 0x43, 0x0200, 0x3E00 },
        { 0x35, 0x79, 0x0400, 0x3C00 },
        { 0x03, 0x91, 0x0080, 0x6400 },
        { 0xAB, 0xDC, 0x0080, 0x6E00 },
        { 0x51, 0x9E, 0x0100, 0x4000 },
        { 0xC1, 0x15, 0x0400, 0x2C00 },
        { 0x99, 0x7D, 0x0080, 0x4400 },
    },

    //TH14
    {
        { 0x1B, 0x73, 0x0100, 0x3800 },
        { 0x12, 0x43, 0x0200, 0x3E00 },
        { 0x35, 0x79, 0x0400, 0x3C00 },
        { 0x03, 0x91, 0x0080, 0x6400 },
        { 0xAB, 0xDC, 0x0080, 0x7000 },
        { 0x51, 0x9E, 0x0100, 0x4000 },
        { 0xC1, 0x15, 0x0400, 0x2C00 },
        { 0x99, 0x7D, 0x0080, 0x4400 }
    },
};

static std::string decompress(
    io::IO &io, size_t size_compressed, size_t size_original)
{
    util::pack::LzssSettings settings;
    settings.position_bits = 13;
    settings.length_bits = 4;
    settings.min_match_length = 3;
    settings.initial_dictionary_pos = 1;
    settings.reuse_compressed = true;

    io::BufferedIO buffered_io;
    buffered_io.write_from_io(io, size_compressed);
    buffered_io.seek(0);
    io::BitReader bit_reader(buffered_io);
    return util::pack::lzss_decompress(bit_reader, size_original, settings);
}

static std::unique_ptr<Header> read_header(io::IO &arc_io)
{
    std::unique_ptr<Header> header(new Header);
    io::BufferedIO header_io;
    decrypt(arc_io, 16, header_io, { 0x1B, 0x37, 0x10, 0x400 });
    if (header_io.read(magic.size()) != magic)
        throw std::runtime_error("Not a THA1 archive");
    header->table_size_original = header_io.read_u32_le() - 123456789;
    header->table_size_compressed = header_io.read_u32_le() - 987654321;
    header->file_count = header_io.read_u32_le() - 135792468;
    header->table_offset = arc_io.size() - header->table_size_compressed;
    return header;
}

static std::unique_ptr<io::BufferedIO> read_raw_table(
    io::IO &arc_io, const Header &header)
{
    io::BufferedIO decrypted_io;
    arc_io.seek(header.table_offset);
    decrypt(
        arc_io,
        header.table_size_compressed,
        decrypted_io,
        { 0x3E, 0x9B, 0x80, header.table_size_compressed });

    return std::unique_ptr<io::BufferedIO>(
        new io::BufferedIO(
            decompress(
                decrypted_io,
                header.table_size_compressed,
                header.table_size_original)));
}

static Table read_table(io::IO &arc_io, const Header &header)
{
    Table table;
    auto table_io = read_raw_table(arc_io, header);

    for (auto i : util::range(header.file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);

        entry->name = table_io->read_until_zero();
        table_io->skip(3 - entry->name.length() % 4);

        entry->decryptor_id = 0;
        for (auto j : util::range(entry->name.length()))
            entry->decryptor_id += entry->name[j];
        entry->decryptor_id %= 8;

        entry->offset = table_io->read_u32_le();
        entry->size_original = table_io->read_u32_le();
        table_io->skip(4);
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
    io::IO &arc_io, const TableEntry &entry, int encryption_version)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;

    arc_io.seek(entry.offset);
    io::BufferedIO decrypted_io;
    decrypt(
        arc_io,
        entry.size_compressed,
        decrypted_io,
        decryptors[encryption_version][entry.decryptor_id]);

    if (entry.size_compressed == entry.size_original)
    {
        file->io.write_from_io(decrypted_io, entry.size_original);
    }
    else
    {
        file->io.write(decompress(
            decrypted_io,
            entry.size_compressed,
            entry.size_original));

        if (file->io.size() != entry.size_original)
            throw std::runtime_error("Badly compressed stream");
    }

    return file;
}

static int detect_encryption_version(File &arc_file, const Table &table)
{
    if (arc_file.name.find("th095.") != std::string::npos) return 0;
    if (arc_file.name.find("th10.") != std::string::npos) return 0;
    if (arc_file.name.find("th11.") != std::string::npos) return 0;
    if (arc_file.name.find("th12.") != std::string::npos) return 1;
    if (arc_file.name.find("th125.") != std::string::npos) return 1;
    if (arc_file.name.find("th128.") != std::string::npos) return 1;
    if (arc_file.name.find("th13.") != std::string::npos) return 2;
    if (arc_file.name.find("th14.") != std::string::npos) return 3;
    if (arc_file.name.find("th143.") != std::string::npos) return 3;
    if (arc_file.name.find("th15tr.") != std::string::npos) return 3;
    return -1;
}

struct Tha1Archive::Priv
{
    AnmArchive anm_archive;
};

Tha1Archive::Tha1Archive() : p(new Priv)
{
    add_transformer(&p->anm_archive);
}

Tha1Archive::~Tha1Archive()
{
}

bool Tha1Archive::is_recognized_internal(File &arc_file) const
{
    if (!arc_file.has_extension("dat"))
        return false;
    auto header = read_header(arc_file.io);
    auto table = read_table(arc_file.io, *header);
    return detect_encryption_version(arc_file, table) >= 0;
}

void Tha1Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto header = read_header(arc_file.io);
    auto table = read_table(arc_file.io, *header);
    auto encryption_version = detect_encryption_version(arc_file, table);
    if (encryption_version == -1)
        throw std::runtime_error("Unknown encryption version");

    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry, encryption_version));
}
