#include "fmt/touhou/tha1_archive_decoder.h"
#include "err.h"
#include "fmt/touhou/anm_archive_decoder.h"
#include "fmt/touhou/crypt.h"
#include "io/buffered_io.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

static const bstr magic = "THA1"_b;

namespace
{
    struct Header final
    {
        size_t file_count;
        size_t table_offset;
        size_t table_size_orig;
        size_t table_size_comp;
    };

    struct TableEntry final
    {
        std::string name;
        size_t offset;
        size_t size_comp;
        size_t size_orig;
        u8 decryptor_id;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static std::vector<std::vector<DecryptorContext>> decryptors
{
    // TH9.5, TH10, TH11
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

    // TH12, TH12.5, TH12.8
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

    // TH13
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

    // TH14, TH15 trial, TH15
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

static bstr decompress(const bstr &input, size_t size_orig)
{
    util::pack::LzssSettings settings;
    settings.position_bits = 13;
    settings.size_bits = 4;
    settings.min_match_size = 3;
    settings.initial_dictionary_pos = 1;
    return util::pack::lzss_decompress_bitwise(input, size_orig, settings);
}

static Header read_header(io::IO &arc_io)
{
    arc_io.seek(0);
    auto header_data = arc_io.read(16);
    header_data = decrypt(header_data, { 0x1B, 0x37, 0x10, 0x400 });
    io::BufferedIO header_io(header_data);
    if (header_io.read(magic.size()) != magic)
        throw err::RecognitionError();
    Header header;
    header.table_size_orig = header_io.read_u32_le() - 123456789;
    header.table_size_comp = header_io.read_u32_le() - 987654321;
    header.file_count = header_io.read_u32_le() - 135792468;
    header.table_offset = arc_io.size() - header.table_size_comp;
    return header;
}

static std::unique_ptr<io::IO> read_raw_table(
    io::IO &arc_io, const Header &header)
{
    arc_io.seek(header.table_offset);
    auto table_data = arc_io.read(header.table_size_comp);
    table_data = decrypt(
        table_data, { 0x3E, 0x9B, 0x80, header.table_size_comp });
    table_data = decompress(table_data, header.table_size_orig);
    return std::unique_ptr<io::IO>(new io::BufferedIO(table_data));
}

static Table read_table(io::IO &arc_io, const Header &header)
{
    Table table;
    auto table_io = read_raw_table(arc_io, header);

    for (auto i : util::range(header.file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);

        entry->name = table_io->read_to_zero().str();
        table_io->skip(3 - entry->name.size() % 4);

        entry->decryptor_id = 0;
        for (auto j : util::range(entry->name.size()))
            entry->decryptor_id += entry->name[j];
        entry->decryptor_id %= 8;

        entry->offset = table_io->read_u32_le();
        entry->size_orig = table_io->read_u32_le();
        table_io->skip(4);
        table.push_back(std::move(entry));
    }

    for (auto i : util::range(table.size() - 1))
        table[i]->size_comp = table[i + 1]->offset - table[i]->offset;

    if (table.size() > 0)
    {
        table[table.size() - 1]->size_comp
            = header.table_offset - table[table.size() - 1]->offset;
    }

    return table;
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io, const TableEntry &entry, int encryption_version)
{
    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size_comp);
    data = decrypt(data, decryptors[encryption_version][entry.decryptor_id]);
    if (entry.size_comp != entry.size_orig)
        data = decompress(data, entry.size_orig);

    std::unique_ptr<File> output_file(new File);
    output_file->name = entry.name;
    output_file->io.write(data);
    return output_file;
}

static int detect_encryption_version(File &arc_file)
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
    if (arc_file.name.find("th15.") != std::string::npos) return 3;
    return -1;
}

struct Tha1ArchiveDecoder::Priv final
{
    AnmArchiveDecoder anm_archive_decoder;
};

Tha1ArchiveDecoder::Tha1ArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->anm_archive_decoder);
}

Tha1ArchiveDecoder::~Tha1ArchiveDecoder()
{
}

bool Tha1ArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    if (!arc_file.has_extension("dat"))
        return false;
    return detect_encryption_version(arc_file) >= 0;
}

void Tha1ArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    auto header = read_header(arc_file.io);
    auto table = read_table(arc_file.io, header);
    auto encryption_version = detect_encryption_version(arc_file);
    for (auto &entry : table)
        saver.save(read_file(arc_file.io, *entry, encryption_version));
}

static auto dummy = fmt::Registry::add<Tha1ArchiveDecoder>("th/tha1");
