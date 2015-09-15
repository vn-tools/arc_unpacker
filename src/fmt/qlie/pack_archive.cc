// PACK archive
//
// Company:   -
// Engine:    QLiE
// Extension: .pack
//
// Known games:
// - [Etude] [071122] Soshite Ashita no Sekai yori
// - [Eufonie] [121221] Koiken Otome
// - [Front Wing] [070323] Kimihagu Master

#include <cstring>
#include "err.h"
#include "fmt/qlie/abmp10_archive.h"
#include "fmt/qlie/abmp7_archive.h"
#include "fmt/qlie/dpng_converter.h"
#include "fmt/qlie/mt.h"
#include "fmt/qlie/pack_archive.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::qlie;

static const bstr magic = "FilePackVer3.0\x00\x00"_b;
static const bstr compression_magic = "1PC\xFF"_b;
static const bstr exe_key_magic = "\x05TIcon\x00"_b;

namespace
{
    enum EncryptionType
    {
        Basic = 1,
        WithFKey = 2,
        WithGameExe = 3,
    };

    struct TableEntry final
    {
        bstr name;
        size_t size_compressed;
        size_t size_original;
        size_t offset;
        bool encrypted;
        bool compressed;
        u32 seed;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static u64 padb(u64 a, u64 b)
{
    return ((a & 0x7F7F7F7F7F7F7F7F)
        + (b & 0x7F7F7F7F7F7F7F7F))
        ^ ((a ^ b) & 0x8080808080808080);
}

static u64 padw(u64 a, u64 b)
{
    return ((a & 0x7FFF7FFF7FFF7FFF)
        + (b & 0x7FFF7FFF7FFF7FFF))
        ^ ((a ^ b) & 0x8000800080008000);
}

static u64 padd(u64 a, u64 b)
{
    return ((a & 0x7FFFFFFF7FFFFFFF)
        + (b & 0x7FFFFFFF7FFFFFFF))
        ^ ((a ^ b) & 0x8000000080000000);
}

static size_t get_magic_start(const io::IO &arc_io)
{
    return arc_io.size() - magic.size() - 8 - 4;
}

static u32 derive_seed(const bstr &input)
{
    u64 key = 0;
    u64 result = 0;
    for (auto i : util::range(input.size() >> 3))
    {
        key = padw(key, 0x0307030703070307);
        result = padw(result, input.get<u64>()[i] ^ key);
    }
    result ^= (result >> 32);
    return static_cast<u32>(result & 0xFFFFFFFF);
}

static void decrypt_file_name(bstr &file_name, u32 key)
{
    u8 x = ((key ^ 0x3E) + file_name.size()) & 0xFF;
    for (auto i : util::range(1, file_name.size() + 1))
        file_name.get<u8>()[i - 1] ^= ((i ^ x) & 0xFF) + i;
}

static void decrypt_file_data_basic(bstr &data, u32 seed)
{
    u64 *current = data.get<u64>();
    const u64 *end = current + data.size() / 8;
    u64 key = 0xA73C5F9DA73C5F9D;
    u64 mutator = (seed + data.size()) ^ 0xFEC9753E;
    mutator = (mutator << 32) | mutator;

    while (current < end)
    {
        key = padd(key, 0xCE24F523CE24F523);
        key ^= mutator;
        mutator = *current++ ^= key;
    }
}

static void decrypt_file_data_with_external_keys(
    bstr &data,
    u32 seed,
    EncryptionType enc_type,
    const bstr &file_name,
    const bstr &key1,
    const bstr &key2)
{
    u32 mt_mutator = 0x85F532;
    u32 mt_seed = 0x33F641;

    for (auto i : util::range(file_name.size()))
    {
        mt_mutator += file_name.get<const u8>()[i] * static_cast<u8>(i);
        mt_seed ^= mt_mutator;
    }

    mt_seed += seed ^ (7 * (data.size() & 0xFFFFFF)
        + data.size()
        + mt_mutator
        + (mt_mutator ^ data.size() ^ 0x8F32DC));
    mt_seed = 9 * (mt_seed & 0xFFFFFF);

    if (enc_type == EncryptionType::WithGameExe)
        mt_seed ^= 0x453A;

    CustomMersenneTwister mt(mt_seed);
    mt.xor_state(key1);
    mt.xor_state(key2);

    std::vector<u64> table(16);
    for (auto i : util::range(table.size()))
    {
        table[i]
            = mt.get_next_integer()
            | (static_cast<u64>(mt.get_next_integer()) << 32);
    }
    for (auto i : util::range(9))
         mt.get_next_integer();

    u64 mutator
        = mt.get_next_integer()
        | (static_cast<u64>(mt.get_next_integer()) << 32);

    auto table_index = mt.get_next_integer() % table.size();
    auto data_ptr = data.get<u64>();
    auto data_end = data.end<const u64>();
    while (data_ptr < data_end)
    {
        mutator ^= table[table_index];
        mutator = padd(mutator, table[table_index]);

        *data_ptr ^= mutator;

        mutator = padb(mutator, *data_ptr);
        mutator ^= *data_ptr;
        mutator <<= 1;
        mutator &= 0xFFFFFFFEFFFFFFFE;
        mutator = padw(mutator, *data_ptr);

        table_index++;
        table_index %= table.size();
        data_ptr++;
    }
}

static void decrypt_file_data(
    bstr &data,
    u32 seed,
    EncryptionType enc_type,
    const bstr &file_name,
    const bstr &key1,
    const bstr &key2)
{
    switch (enc_type)
    {
        case EncryptionType::Basic:
            decrypt_file_data_basic(data, seed);
            break;

        case EncryptionType::WithFKey:
        case EncryptionType::WithGameExe:
            decrypt_file_data_with_external_keys(
                data, seed, enc_type, file_name, key1, key2);
            break;
    }
}

static bstr decompress(const bstr &input, size_t output_size)
{
    bstr output(output_size);
    char *output_ptr = output.get<char>();
    const char *output_end = output_ptr + output_size;

    io::BufferedIO input_io(input);

    if (input_io.read(compression_magic.size()) != compression_magic)
    {
        throw err::CorruptDataError(
            "Unexpected magic in compressed file. "
            "Try with --fkey or --gameexe?");
    }

    bool use_short_size = input_io.read_u32_le() > 0;
    u32 file_size = input_io.read_u32_le();
    if (file_size != output_size)
        throw err::BadDataSizeError();

    u8 dict1[256];
    u8 dict2[256];
    u8 dict3[256];
    while (!input_io.eof())
    {
        for (auto i : util::range(256))
            dict1[i] = i;

        for (size_t d = 0; d < 256; )
        {
            u8 c = input_io.read_u8();
            if (c > 0x7F)
            {
                d += c - 0x7F;
                c = 0;
                if (d >= 256)
                    break;
            }

            for (auto i : util::range(c + 1))
            {
                dict1[d] = input_io.read_u8();
                if (dict1[d] != d)
                    dict2[d] = input_io.read_u8();
                d++;
            }
        }

        int bytes_left = use_short_size
            ? input_io.read_u16_le()
            : input_io.read_u32_le();

        u8 n = 0;
        while (true)
        {
            u8 d;
            if (n > 0)
            {
                d = dict3[--n];
            }
            else
            {
                if (bytes_left == 0)
                    break;
                bytes_left--;
                d = input_io.read_u8();
            }

            if (dict1[d] == d)
            {
                *output_ptr++ = d;
                if (output_ptr >= output_end)
                    return output;
            }
            else
            {
                dict3[n++] = dict2[d];
                dict3[n++] = dict1[d];
            }
        }
    }
    return output;
}

static bstr read_file_name(io::IO &table_io, u32 key)
{
    size_t file_name_size = table_io.read_u16_le();
    bstr file_name = table_io.read(file_name_size);
    decrypt_file_name(file_name, key);
    return file_name;
}

static Table read_table(io::IO &arc_io)
{
    arc_io.seek(get_magic_start(arc_io) + magic.size());
    auto file_count = arc_io.read_u32_le();
    auto table_offset = arc_io.read_u64_le();
    auto table_size = get_magic_start(arc_io) - table_offset;
    arc_io.seek(table_offset);

    io::BufferedIO table_io;
    table_io.write_from_io(arc_io, table_size);

    u32 seed = 0;
    table_io.seek(0);
    for (auto i : util::range(file_count))
    {
        size_t file_name_size = table_io.read_u16_le();
        table_io.skip(file_name_size + 28);
    }
    table_io.skip(28);
    table_io.skip(table_io.read_u32_le());
    table_io.skip(36);
    seed = derive_seed(table_io.read(256)) & 0x0FFFFFFF;

    Table table;
    table_io.seek(0);
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);

        entry->name = read_file_name(table_io, seed);
        entry->offset = table_io.read_u64_le();
        entry->size_compressed = table_io.read_u32_le();
        entry->size_original = table_io.read_u32_le();
        entry->compressed = table_io.read_u32_le() > 0;
        entry->encrypted = table_io.read_u32_le() > 0;

        entry->seed = seed;
        table_io.skip(4);

        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io,
    const TableEntry &entry,
    EncryptionType enc_type,
    const bstr key1,
    const bstr key2)
{
    std::unique_ptr<File> file(new File);
    file->name = util::sjis_to_utf8(entry.name).str();

    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size_compressed);

    if (entry.encrypted)
        decrypt_file_data(data, entry.seed, enc_type, entry.name, key1, key2);

    if (entry.compressed)
        data = decompress(data, entry.size_original);

    file->io.write(data);
    return file;
}

struct PackArchive::Priv final
{
    Priv();

    DpngConverter dpng_converter;
    Abmp7Archive abmp7_archive;
    Abmp10Archive abmp10_archive;

    EncryptionType enc_type;
    bstr key1;
    bstr key2;
};

PackArchive::Priv::Priv() : enc_type(EncryptionType::Basic)
{
}

PackArchive::PackArchive() : p(new Priv)
{
    add_transformer(&p->dpng_converter);
    add_transformer(&p->abmp7_archive);
    add_transformer(&p->abmp10_archive);
}

PackArchive::~PackArchive()
{
}

void PackArchive::register_cli_options(ArgParser &arg_parser) const
{
    arg_parser.register_switch({"--fkey"})
        ->set_value_name("PATH")
        ->set_description("Selects path to fkey file");

    arg_parser.register_switch({"--gameexe"})
        ->set_value_name("PATH")
        ->set_description("Selects path to game executable");

    Archive::register_cli_options(arg_parser);
}

void PackArchive::parse_cli_options(const ArgParser &arg_parser)
{
    if (arg_parser.has_switch("fkey"))
    {
        const std::string file_path = arg_parser.get_switch("fkey");
        p->enc_type = EncryptionType::WithFKey;
        File file(file_path, io::FileMode::Read);
        p->key1 = file.io.read_to_eof();
    }

    if (arg_parser.has_switch("gameexe"))
    {
        if (!arg_parser.has_switch("fkey"))
            throw err::UsageError("Must specify also --fkey.");

        static const int key2_size = 256;
        const std::string path = arg_parser.get_switch("gameexe");

        File file(path, io::FileMode::Read);
        auto exe_data = file.io.read_to_eof();

        bool found = false;
        auto start = file.io.size() - exe_key_magic.size() - key2_size;
        for (auto i : util::range(start, 0, -1))
        {
            if (!std::memcmp(&exe_data[i],
                exe_key_magic.get<char>(),
                exe_key_magic.size()))
            {
                found = true;
                p->enc_type = EncryptionType::WithGameExe;
                p->key2 = bstr(
                    &exe_data[i + exe_key_magic.size() - 1], key2_size);
            }
        }

        if (!found)
            throw err::RecognitionError("Cannot find the key in the .exe file");
    }

    Archive::parse_cli_options(arg_parser);
}

bool PackArchive::is_recognized_internal(File &arc_file) const
{
    arc_file.io.seek(get_magic_start(arc_file.io));
    return arc_file.io.read(magic.size()) == magic;
}

void PackArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(
            arc_file.io, *entry, p->enc_type, p->key1, p->key2);
        if (file->name.find("pack_keyfile") != std::string::npos)
        {
            file->io.seek(0);
            p->key1 = file->io.read_to_eof();
        }
        file_saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<PackArchive>("qlie/pack");
