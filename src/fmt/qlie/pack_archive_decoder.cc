#include "fmt/qlie/pack_archive_decoder.h"
#include <cstring>
#include "err.h"
#include "fmt/qlie/mt.h"
#include "io/memory_stream.h"
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

    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        EncryptionType enc_type;
        bstr key1;
        bstr key2;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        bstr path_orig;
        size_t size_compressed;
        size_t size_original;
        size_t offset;
        bool encrypted;
        bool compressed;
        u32 seed;
    };
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

static size_t get_magic_start(const io::Stream &arc_stream)
{
    return arc_stream.size() - magic.size() - 8 - 4;
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
    bstr &data, u32 seed, const bstr &file_name, const ArchiveMetaImpl &meta)
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

    if (meta.enc_type == EncryptionType::WithGameExe)
        mt_seed ^= 0x453A;

    CustomMersenneTwister mt(mt_seed);
    mt.xor_state(meta.key1);
    mt.xor_state(meta.key2);

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
    const bstr &file_name,
    const ArchiveMetaImpl &meta)
{
    switch (meta.enc_type)
    {
        case EncryptionType::Basic:
            decrypt_file_data_basic(data, seed);
            break;

        case EncryptionType::WithFKey:
        case EncryptionType::WithGameExe:
            decrypt_file_data_with_external_keys(
                data, seed, file_name, meta);
            break;
    }
}

static bstr decompress(const bstr &input, size_t output_size)
{
    bstr output(output_size);
    char *output_ptr = output.get<char>();
    const char *output_end = output_ptr + output_size;

    io::MemoryStream input_stream(input);

    if (input_stream.read(compression_magic.size()) != compression_magic)
    {
        throw err::CorruptDataError(
            "Unexpected magic in compressed file. "
            "Try with --fkey or --gameexe?");
    }

    bool use_short_size = input_stream.read_u32_le() > 0;
    u32 file_size = input_stream.read_u32_le();
    if (file_size != output_size)
        throw err::BadDataSizeError();

    u8 dict1[256];
    u8 dict2[256];
    u8 dict3[256];
    while (!input_stream.eof())
    {
        for (auto i : util::range(256))
            dict1[i] = i;

        for (size_t d = 0; d < 256; )
        {
            u8 c = input_stream.read_u8();
            if (c > 0x7F)
            {
                d += c - 0x7F;
                c = 0;
                if (d >= 256)
                    break;
            }

            for (auto i : util::range(c + 1))
            {
                dict1[d] = input_stream.read_u8();
                if (dict1[d] != d)
                    dict2[d] = input_stream.read_u8();
                d++;
            }
        }

        int bytes_left = use_short_size
            ? input_stream.read_u16_le()
            : input_stream.read_u32_le();

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
                d = input_stream.read_u8();
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

struct PackArchiveDecoder::Priv final
{
    std::string fkey_path;
    std::string game_exe_path;
};

PackArchiveDecoder::PackArchiveDecoder() : p(new Priv)
{
}

PackArchiveDecoder::~PackArchiveDecoder()
{
}

void PackArchiveDecoder::register_cli_options(ArgParser &arg_parser) const
{
    arg_parser.register_switch({"--fkey"})
        ->set_value_name("PATH")
        ->set_description("Selects path to fkey file");

    arg_parser.register_switch({"--gameexe"})
        ->set_value_name("PATH")
        ->set_description("Selects path to game executable");

    ArchiveDecoder::register_cli_options(arg_parser);
}

void PackArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    if (arg_parser.has_switch("fkey"))
        p->fkey_path = arg_parser.get_switch("fkey");

    if (arg_parser.has_switch("gameexe"))
    {
        if (!arg_parser.has_switch("fkey"))
            throw err::UsageError("Must specify also --fkey.");
        p->game_exe_path = arg_parser.get_switch("gameexe");
    }

    ArchiveDecoder::parse_cli_options(arg_parser);
}

bool PackArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(get_magic_start(input_file.stream));
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    PackArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMetaImpl>();
    meta->enc_type = EncryptionType::Basic;

    if (!p->fkey_path.empty())
    {
        meta->enc_type = EncryptionType::WithFKey;
        io::File file(p->fkey_path, io::FileMode::Read);
        meta->key1 = file.stream.read_to_eof();
    }

    if (!p->game_exe_path.empty())
    {
        static const int key2_size = 256;

        io::File file(p->game_exe_path, io::FileMode::Read);
        auto exe_data = file.stream.read_to_eof();

        bool found = false;
        auto start = file.stream.size() - exe_key_magic.size() - key2_size;
        for (auto i : util::range(start, 0, -1))
        {
            if (!std::memcmp(&exe_data[i],
                exe_key_magic.get<char>(),
                exe_key_magic.size()))
            {
                found = true;
                meta->enc_type = EncryptionType::WithGameExe;
                meta->key2 = bstr(
                    &exe_data[i + exe_key_magic.size() - 1], key2_size);
            }
        }

        if (!found)
            throw err::RecognitionError("Cannot find the key in the .exe file");
    }

    input_file.stream.seek(get_magic_start(input_file.stream) + magic.size());
    auto file_count = input_file.stream.read_u32_le();
    auto table_offset = input_file.stream.read_u64_le();
    auto table_size = get_magic_start(input_file.stream) - table_offset;
    input_file.stream.seek(table_offset);
    io::MemoryStream table_stream(input_file.stream, table_size);

    u32 seed = 0;
    table_stream.seek(0);
    for (auto i : util::range(file_count))
    {
        size_t file_name_size = table_stream.read_u16_le();
        table_stream.skip(file_name_size + 28);
    }
    table_stream.skip(28);
    table_stream.skip(table_stream.read_u32_le());
    table_stream.skip(36);
    seed = derive_seed(table_stream.read(256)) & 0x0FFFFFFF;

    table_stream.seek(0);
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();

        size_t name_size = table_stream.read_u16_le();
        entry->path_orig = table_stream.read(name_size);
        decrypt_file_name(entry->path_orig, seed);
        entry->path = util::sjis_to_utf8(entry->path_orig).str();

        entry->offset = table_stream.read_u64_le();
        entry->size_compressed = table_stream.read_u32_le();
        entry->size_original = table_stream.read_u32_le();
        entry->compressed = table_stream.read_u32_le() > 0;
        entry->encrypted = table_stream.read_u32_le() > 0;

        entry->seed = seed;
        table_stream.skip(4);

        meta->entries.push_back(std::move(entry));
    }

    for (auto &entry : meta->entries)
    {
        if (entry->path.name().find("pack_keyfile") != std::string::npos)
        {
            auto file = read_file(input_file, *meta, *entry);
            file->stream.seek(0);
            meta->key1 = file->stream.read_to_eof();
        }
    }

    return std::move(meta);
}

std::unique_ptr<io::File> PackArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size_compressed);

    if (entry->encrypted)
        decrypt_file_data(data, entry->seed, entry->path_orig, *meta);

    if (entry->compressed)
        data = decompress(data, entry->size_original);

    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> PackArchiveDecoder::get_linked_formats() const
{
    return {"qlie/abmp7", "qlie/abmp10", "qlie/dpng"};
}

static auto dummy = fmt::register_fmt<PackArchiveDecoder>("qlie/pack");
