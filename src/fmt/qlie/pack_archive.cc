// PACK archive
//
// Company:   -
// Engine:    QLiE
// Extension: .pack
//
// Known games:
// - Koiken Otome
// - Soshite Ashita no Sekai yori
// - Kimihagu

#include <cstring>
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

namespace
{
    enum EncryptionType
    {
        Basic = 1,
        WithFKey = 2,
        WithGameExe = 3,
    };

    struct TableEntry
    {
        std::string orig_name;
        std::string name;
        size_t size_compressed;
        size_t size_original;
        size_t offset;
        bool encrypted;
        bool compressed;
        u32 seed;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static const std::string magic1 = "FilePackVer1.0\x00\x00"_s;
static const std::string magic2 = "FilePackVer2.0\x00\x00"_s;
static const std::string magic3 = "FilePackVer3.0\x00\x00"_s;

static int guess_version(io::IO &arc_io)
{
    int version = 1;
    for (auto &magic : {magic1, magic2, magic3})
    {
        arc_io.seek(arc_io.size() - 0x1C);
        if (arc_io.read(magic.size()) == magic)
            return version;
        ++version;
    }
    return -1;
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

static u32 v3_derive_seed(io::IO &io, size_t bytes)
{
    u64 key = 0;
    u64 result = 0;
    size_t size = bytes / 8;
    std::unique_ptr<u64[]> input(new u64[size]);
    u64 *input_ptr = input.get();
    u64 *input_guardian = input_ptr + size;
    io.read(input_ptr, bytes);

    while (input_ptr < input_guardian)
    {
        key = padw(key, 0x0307030703070307);
        result = padw(result, *input_ptr++ ^ key);
    }
    result ^= (result >> 32);
    return static_cast<u32>(result & 0xFFFFFFFF);
}

static void v3_decrypt_file_name(u8 *file_name, size_t length, u32 key)
{
    u8 _xor = ((key ^ 0x3E) + length) & 0xFF;
    for (size_t i = 1; i <= length; i++)
        file_name[i - 1] ^= ((i ^ _xor) & 0xFF) + i;
}

static void v3_decrypt_file_data_basic(u8 *file_data, size_t length, u32 seed)
{
    u64 *current = reinterpret_cast<u64*>(file_data);
    const u64 *end = current + length / 8;
    u64 key = 0xA73C5F9DA73C5F9D;
    u64 mutator = (seed + length) ^ 0xFEC9753E;
    mutator = (mutator << 32) | mutator;

    while (current < end)
    {
        key = padd(key, 0xCE24F523CE24F523);
        key ^= mutator;
        mutator = *current++ ^= key;
    }
}

static void v3_decrypt_file_data_with_external_keys(
    u8 *file_data,
    size_t length,
    u32 seed,
    EncryptionType encryption_type,
    const std::string file_name,
    const std::string key1,
    const std::string key2)
{
    u64 *current = reinterpret_cast<u64*>(file_data);
    const u64 *end = current + length / 8;
    unsigned long mt_mutator = 0x85F532;
    unsigned long mt_seed = 0x33F641;

    for (auto i : util::range(file_name.length()))
    {
        mt_mutator += static_cast<const u8&>(file_name[i]) * static_cast<u8>(i);
        mt_seed ^= mt_mutator;
    }

    mt_seed += seed ^ (7 * (length & 0xFFFFFF)
        + length
        + mt_mutator
        + (mt_mutator ^ length ^ 0x8F32DC));
    mt_seed = 9 * (mt_seed & 0xFFFFFF);

    if (encryption_type == EncryptionType::WithGameExe)
        mt_seed ^= 0x453A;

    mt::init_genrand(mt_seed);
    mt::xor_state(reinterpret_cast<const u8 *>(key1.data()), key1.size());
    mt::xor_state(reinterpret_cast<const u8 *>(key2.data()), key2.size());

    u64 table[0x10] = { 0 };
    for (auto i : util::range(0x10))
    {
        table[i]
            = mt::genrand_int32()
            | (static_cast<u64>(mt::genrand_int32()) << 32);
    }
    for (auto i : util::range(9))
         mt::genrand_int32();

    u64 mutator
        = mt::genrand_int32()
        | (static_cast<u64>(mt::genrand_int32()) << 32);

    size_t table_index = mt::genrand_int32() & 0xF;
    while (current < end)
    {
        mutator ^= table[table_index];
        mutator = padd(mutator, table[table_index]);

        *current ^= mutator;

        mutator = padb(mutator, *current);
        mutator ^= *current;
        mutator <<= 1;
        mutator &= 0xFFFFFFFEFFFFFFFE;
        mutator = padw(mutator, *current);

        table_index++;
        table_index &= 0xF;
        current++;
    }
}

static void v3_decrypt_file_data(
    u8 *file_data,
    size_t length,
    u32 seed,
    EncryptionType encryption_type,
    const std::string file_name,
    const std::string key1,
    const std::string key2)
{
    switch (encryption_type)
    {
        case EncryptionType::Basic:
            v3_decrypt_file_data_basic(file_data, length, seed);
            break;

        case EncryptionType::WithFKey:
        case EncryptionType::WithGameExe:
            v3_decrypt_file_data_with_external_keys(
                file_data,
                length,
                seed,
                encryption_type,
                file_name,
                key1,
                key2);
            break;
    }
}

static void decompress(
    const char *input,
    size_t input_size,
    char *output,
    size_t output_size)
{
    char *output_ptr = output;
    const char *output_guardian = output + output_size;

    io::BufferedIO input_io(input, input_size);

    std::string magic = "1PC\xFF"_s;
    if (input_io.read(magic.length()) != magic)
    {
        throw std::runtime_error("Unexpected magic in compressed file. "
            "Try with --fkey or --gameexe?");
    }

    bool use_short_length = input_io.read_u32_le() > 0;
    u32 file_size = input_io.read_u32_le();
    if (file_size != output_size)
        throw std::runtime_error("Unpexpected file size");

    u8 dict1[256];
    u8 dict2[256];
    u8 dict3[256];
    while (input_io.tell() < input_io.size())
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

        int bytes_left = use_short_length
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
                if (output_ptr >= output_guardian)
                    return;
            }
            else
            {
                dict3[n++] = dict2[d];
                dict3[n++] = dict1[d];
            }
        }
    }
}

static std::string read_file_name(io::IO &table_io, u32 key, int version)
{
    size_t file_name_length = table_io.read_u16_le();
    std::unique_ptr<char[]> file_name(new char[file_name_length + 1]);
    table_io.read(file_name.get(), file_name_length);
    if (version != 3)
        throw std::runtime_error("Not implemented");
    v3_decrypt_file_name(
        reinterpret_cast<u8*>(file_name.get()), file_name_length, key);
    return std::string(file_name.get(), file_name_length);
}

static Table read_table(io::IO &arc_io, int version)
{
    size_t file_count = arc_io.read_u32_le();
    u64 table_offset = arc_io.read_u64_le();
    size_t table_size = (arc_io.size() - 0x1C) - table_offset;
    arc_io.seek(table_offset);

    io::BufferedIO table_io;
    table_io.write_from_io(arc_io, table_size);

    u32 seed = 0;
    if (version == 3)
    {
        table_io.seek(0);
        for (auto i : util::range(file_count))
        {
            size_t file_name_length = table_io.read_u16_le();
            table_io.skip(file_name_length + 28);
        }
        table_io.skip(28);
        table_io.skip(table_io.read_u32_le());
        table_io.skip(36);
        seed = v3_derive_seed(table_io, 256) & 0x0FFFFFFF;
    }

    Table table;
    table_io.seek(0);
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);

        entry->orig_name = read_file_name(table_io, seed, version);
        entry->name = util::sjis_to_utf8(entry->orig_name);
        entry->offset = table_io.read_u64_le();
        entry->size_compressed = table_io.read_u32_le();
        entry->size_original = table_io.read_u32_le();
        entry->compressed = table_io.read_u32_le() > 0;
        entry->encrypted = table_io.read_u32_le() > 0;

        if (version == 1 /* || version == 2? */)
        {
            entry->seed = table_io.read_u32_le();
        }
        else
        {
            entry->seed = seed;
            table_io.skip(4);
        }

        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io,
    const TableEntry &entry,
    int version,
    EncryptionType encryption_type,
    const std::string key1,
    const std::string key2)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;

    arc_io.seek(entry.offset);
    std::unique_ptr<char[]> data(new char[entry.size_compressed]);
    arc_io.read(data.get(), entry.size_compressed);

    if (entry.encrypted)
    {
        if (version == 3)
        {
            v3_decrypt_file_data(
                reinterpret_cast<u8*>(data.get()),
                entry.size_compressed,
                entry.seed,
                encryption_type,
                entry.orig_name,
                key1,
                key2);
        }
        else
            throw std::runtime_error("Not implemented");
    }

    if (entry.compressed)
    {
        std::unique_ptr<char[]> decompressed(new char[entry.size_original]);

        decompress(
            data.get(),
            entry.size_compressed,
            decompressed.get(),
            entry.size_original);

        data = std::move(decompressed);
    }

    file->io.write(data.get(), entry.size_original);

    return file;
}

struct PackArchive::Priv
{
    DpngConverter dpng_converter;
    Abmp7Archive abmp7_archive;
    Abmp10Archive abmp10_archive;

    EncryptionType encryption_type;
    std::string key1;
    std::string key2;

    Priv()
    {
        encryption_type = EncryptionType::Basic;
    }
};

PackArchive::PackArchive() : p(new Priv)
{
    add_transformer(&p->dpng_converter);
    add_transformer(&p->abmp7_archive);
    add_transformer(&p->abmp10_archive);
}

PackArchive::~PackArchive()
{
}

void PackArchive::add_cli_help(ArgParser &arg_parser) const
{
    arg_parser.add_help(
        "--fkey=PATH", "Selects path to fkey file.\n");

    arg_parser.add_help(
        "--gameexe=PATH", "Selects path to game executable.\n");

    Archive::add_cli_help(arg_parser);
}

void PackArchive::parse_cli_options(const ArgParser &arg_parser)
{
    if (arg_parser.has_switch("fkey"))
    {
        const std::string file_path = arg_parser.get_switch("fkey");
        File file(file_path, io::FileMode::Read);
        p->encryption_type = EncryptionType::WithFKey;
        p->key1 = file.io.read(file.io.size());
    }

    if (arg_parser.has_switch("gameexe"))
    {
        if (!arg_parser.has_switch("fkey"))
            throw std::runtime_error("Must specify also --fkey.");

        const std::string magic = "\x05TIcon\x00"_s;
        const std::string path = arg_parser.get_switch("gameexe");

        File file(path, io::FileMode::Read);
        std::unique_ptr<char[]> exe_data(new char[file.io.size()]);
        file.io.read(exe_data.get(), file.io.size());

        bool found = false;
        for (size_t i = file.io.size() - magic.length(); i > 0; i--)
        {
            if (!memcmp(&exe_data[i], magic.data(), magic.length()))
            {
                found = true;
                p->encryption_type = EncryptionType::WithGameExe;
                p->key2 = std::string(&exe_data[i + magic.length() - 1], 256);
            }
        }
        if (!found)
            throw std::runtime_error("Cannot find the key in the .exe file");
    }

    Archive::parse_cli_options(arg_parser);
}

bool PackArchive::is_recognized_internal(File &arc_file) const
{
    return guess_version(arc_file.io) >= 0;
}

void PackArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    size_t version = guess_version(arc_file.io);
    if (version == 1 || version == 2)
        throw std::runtime_error("Version 1 and 2 are not supported.");

    Table table = read_table(arc_file.io, version);
    for (auto &entry : table)
    {
        auto file = read_file(
            arc_file.io,
            *entry,
            version,
            p->encryption_type,
            p->key1,
            p->key2);
        file_saver.save(std::move(file));
    }
}
