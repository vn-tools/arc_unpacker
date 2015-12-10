#include "fmt/purple_software/cpz5_archive_decoder.h"
#include "algo/binary.h"
#include "algo/crypt/md5.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::fmt::purple_software;

static const bstr magic = "CPZ5"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        u32 key;
    };

    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        u32 main_key;
        std::array<u32, 4> pseudo_hash;
    };

    struct Header final
    {
        size_t dir_count;
        size_t dir_table_size;
        size_t file_table_size;
        std::array<u32, 4> md5_dwords;
        u32 main_key;
    };

    struct DirectoryInfo final
    {
        size_t file_count;
        size_t file_table_offset;
        size_t file_table_main_key;
        size_t file_table_size;
        io::path path;
    };
}

static void decrypt_strategy_1a(u8 *target, const size_t size, const u32 key)
{
    static const std::vector<u32> crypt =
    {
        0xCD90F089, 0xE982B782, 0xA282AB88, 0xCD82718E,
        0x52838A83, 0xA882AA82, 0x7592648E, 0xB582AB82,
        0xE182BF82, 0xDC82A282, 0x4281B782, 0x62838183,
        0xA9824981, 0xA282ED82, 0xC682A282, 0xBE8CA982,
        0xC482C182, 0x968BE082, 0xC482B582, 0xB082A082,
        0xA282C882, 0xBE82F182, 0xE782A982, 0x49819F82,
    };

    std::array<u32, 24> table;
    for (const auto i : algo::range(crypt.size()))
        table[i] = crypt[i] - key;

    size_t shift = key;
    for (const auto i : algo::range(3))
        shift = (shift >> 8) ^ key;
    shift = ((shift ^ 0xFB) & 0x0F) + 7;

    size_t table_pos = 5;

    auto target_u32 = reinterpret_cast<u32*>(target);
    for (const auto i : algo::range(size >> 2))
    {
        const auto tmp = (table[table_pos++] ^ *target_u32) + 0x784C5962;
        *target_u32++ = algo::rotr<u32>(tmp, shift) + 0x01010101;
        table_pos %= table.size();
    }

    auto target_u8 = reinterpret_cast<u8*>(target_u32);
    for (auto i : algo::range(size & 3))
    {
        *target_u8
            = ((table[table_pos++] >> (4 * (3 - i))) ^ *target_u8) - 0x79;
        target_u8++;
        table_pos %= table.size();
    }
}

static void decrypt_strategy_1b(
    u8 *target,
    const size_t size,
    const u32 key,
    const std::array<u32, 4> &pseudo_hash)
{
    static const std::array<u32, 4> addends = {0x76A3BF29, 0, 0x10000000, 0};

    std::array<u32, 4> table;
    for (const auto i : algo::range(4))
        table[i] = pseudo_hash[i] ^ (key + addends[i]);

    size_t table_pos = 0;

    auto target_u32 = reinterpret_cast<u32*>(target);
    u32 seed = 0x76548AEF;
    for (const auto i : algo::range(size >> 2))
    {
        u32 tmp = (*target_u32 ^ table[table_pos++]) - 0x4A91C262;
        *target_u32++ = algo::rotl<u32>(tmp, 3) - seed;
        table_pos %= table.size();
        seed += 0x10FB562A;
    }

    auto target_u8 = reinterpret_cast<u8*>(target_u32);
    for (const auto i : algo::range(size & 3))
    {
        *target_u8 = ((table[table_pos++] >> 6) ^ *target_u8) + 0x37;
        target_u8++;
        table_pos %= table.size();
    }
}

static void decrypt_strategy_1c(
    u8 *target,
    const size_t size,
    const u32 key,
    const std::array<u32, 4> &pseudo_hash)
{
    static const std::array<u32, 4> addends = {0, 0x112233, 0, 0x34258765};

    std::array<u32, 4> table;
    for (const auto i : algo::range(4))
        table[i] = pseudo_hash[i] ^ (key + addends[i]);

    size_t table_pos = 0;

    auto target_u32 = reinterpret_cast<u32*>(target);
    u32 seed = 0x2A65CB4E;
    for (const auto i : algo::range(size >> 2))
    {
        u32 tmp = (*target_u32 ^ table[table_pos++]) - seed;
        *target_u32++ = algo::rotl<u32>(tmp, 2) + 0x37A19E8B;
        table_pos %= table.size();
        seed -= 0x139FA9B;
    }

    auto target_u8 = reinterpret_cast<u8*>(target_u32);
    for (const auto i : algo::range(size & 3))
    {
        *target_u8 = ((table[table_pos++] >> 4) ^ *target_u8) + 0x05;
        target_u8++;
        table_pos %= table.size();
    }
}

static std::array<u8, 256> get_table_for_decrypt_strategy_2_and_3(
    u32 key, const u32 seed)
{
    std::array<u8, 256> table;
    for (const auto i : algo::range(256))
        table[i] = i;
    for (const auto i : algo::range(256))
    {
        std::swap(table[(key >> 16) & 0xFF], table[key & 0xFF]);
        std::swap(table[(key >> 8) & 0xFF], table[(key >> 24) & 0xFF]);
        key = seed + algo::rotr<u32>(key, 2) * 0x1A740235;
    }
    return table;
}

static void decrypt_strategy_2(
    u8 *target,
    const size_t size,
    const u32 key,
    const u32 seed,
    const u8 permutation_xor)
{
    const auto table = get_table_for_decrypt_strategy_2_and_3(key, seed);
    for (const auto i : algo::range(size))
        target[i] = table[target[i] ^ permutation_xor];
}

static void decrypt_strategy_3(
    u8 *target,
    const size_t size,
    const u32 key,
    const u32 seed,
    const std::array<u32, 4> &pseudo_hash,
    const u32 entry_key)
{
    const auto table = get_table_for_decrypt_strategy_2_and_3(key, seed);

    static const auto crypt =
        "\x89\xF0\x90\xCD\x82\xB7\x82\xE9\x88\xAB\x82\xA2\x8E\x71\x82\xCD"
        "\x83\x8A\x83\x52\x82\xAA\x82\xA8\x8E\x64\x92\x75\x82\xAB\x82\xB5"
        "\x82\xBF\x82\xE1\x82\xA2\x82\xDC\x82\xB7\x81\x42\x83\x81\x83\x62"
        "\x81\x49\x82\xA9\x82\xED\x82\xA2\x82\xA2\x82\xC6\x82\xA9\x8C\xBE"
        "\x82\xC1\x82\xC4\x82\xE0\x8B\x96\x82\xB5\x82\xC4\x82\xA0\x82\xB0"
        "\x82\xC8\x82\xA2\x82\xF1\x82\xBE\x82\xA9\x82\xE7\x82\x9F\x81\x49"_b;

    u32 yet_another_table[24];
    for (const auto i : algo::range(96))
    {
        reinterpret_cast<u8*>(yet_another_table)[i]
            = table[crypt[i]] ^ (pseudo_hash[1] >> 2);
    }

    for (const auto i : algo::range(24))
        yet_another_table[i] ^= entry_key;

    size_t yet_another_table_pos = 9;
    u32 yet_another_key = 0x2547A39E;

    u32 *target_u32 = reinterpret_cast<u32*>(target);
    for (const auto i : algo::range(size >> 2))
    {
        const auto tmp1 = *target_u32;
        const auto tmp2 = yet_another_table[yet_another_table_pos & 0x0F] >> 1;
        const auto tmp3 = yet_another_table[(yet_another_key >> 6) & 0x0F];
        const auto final_tmp = tmp1 ^ tmp2 ^ tmp3;
        *target_u32 = pseudo_hash[yet_another_key & 3] ^ (final_tmp - entry_key);

        yet_another_table_pos++;
        yet_another_key += *target_u32 + entry_key;
        target_u32++;
    }

    u8 *target_u8 = reinterpret_cast<u8*>(target_u32);
    for (const auto i : algo::range(size & 3))
        target_u8[i] = table[target_u8[i] ^ 0xCB];
}

static Header read_header(io::Stream &input_stream)
{
    input_stream.seek(magic.size());
    Header header;
    header.dir_count = input_stream.read_u32_le() ^ 0xFE3A53D9;
    header.dir_table_size = input_stream.read_u32_le() ^ 0x37F298E7;
    header.file_table_size = input_stream.read_u32_le() ^ 0x7A6F3A2C;
    input_stream.skip(16);
    header.md5_dwords[0] = input_stream.read_u32_le() ^ 0x43DE7C19;
    header.md5_dwords[1] = input_stream.read_u32_le() ^ 0xCC65F415;
    header.md5_dwords[2] = input_stream.read_u32_le() ^ 0xD016A93C;
    header.md5_dwords[3] = input_stream.read_u32_le() ^ 0x97A3BA9A;
    header.main_key = input_stream.read_u32_le() ^ 0xAE7D39BF;
    input_stream.skip(12);
    return header;
}

static std::array<u32, 4> get_pseudo_hash(
    const std::array<u32, 4> &input_dwords)
{
    io::MemoryStream tmp_stream;
    for (const auto &dword : input_dwords)
        tmp_stream.write_u32_le(dword);

    const auto hash_bytes = algo::crypt::md5(
        tmp_stream.seek(0).read_to_eof(),
        {0x53FE9B2C, 0xF2C93EA8, 0xEE81BA59, 0xA2C8973E});

    tmp_stream.seek(0).write(hash_bytes).seek(0);
    std::array<u32, 4> hash_dwords;
    for (const auto i : algo::range(4))
        hash_dwords[i] = tmp_stream.read_u32_le();

    std::array<u32, 4> result;
    result[0] = hash_dwords[1] ^ 0x49875325;
    result[1] = hash_dwords[2] + 0x54F46D7D;
    result[2] = hash_dwords[3] ^ 0xAD7948B7;
    result[3] = hash_dwords[0] + 0x1D0638AD;
    return result;
}

bool Cpz5ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Cpz5ArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    const auto header = read_header(input_file.stream);
    const auto pseudo_hash = get_pseudo_hash(header.md5_dwords);

    const auto table_size = header.dir_table_size + header.file_table_size;
    const auto data_start = input_file.stream.tell() + table_size;
    auto table_data = input_file.stream.read(table_size);
    const auto dir_table_ptr = table_data.get<u8>();
    const auto file_table_ptr = table_data.get<u8>() + header.dir_table_size;

    // whole index
    decrypt_strategy_1a(
        table_data.get<u8>(),
        table_size,
        header.main_key ^ 0x3795B39A);

    // just directories
    decrypt_strategy_2(
        dir_table_ptr,
        header.dir_table_size,
        header.main_key,
        pseudo_hash[1],
        0x3A);
    decrypt_strategy_1b(
        dir_table_ptr,
        header.dir_table_size,
        header.main_key,
        pseudo_hash);

    DirectoryInfo *prev_dir = nullptr;
    std::vector<std::unique_ptr<DirectoryInfo>> dirs;
    io::MemoryStream dir_table_stream(table_data);
    for (const auto i : algo::range(header.dir_count))
    {
        auto dir = std::make_unique<DirectoryInfo>();
        const auto entry_offset = dir_table_stream.tell();
        const auto entry_size = dir_table_stream.read_u32_le();
        dir->file_count = dir_table_stream.read_u32_le();
        dir->file_table_offset = dir_table_stream.read_u32_le();
        dir->file_table_main_key = dir_table_stream.read_u32_le();
        dir->path = algo::sjis_to_utf8(dir_table_stream.read_to_zero()).str();
        if (prev_dir)
        {
            prev_dir->file_table_size
                = dir->file_table_offset - prev_dir->file_table_offset;
        }
        prev_dir = dir.get();
        dirs.push_back(std::move(dir));
        dir_table_stream.seek(entry_offset + entry_size);
    }
    if (prev_dir)
    {
        prev_dir->file_table_size
            = table_size - header.dir_table_size - prev_dir->file_table_offset;
    }

    auto meta = std::make_unique<ArchiveMetaImpl>();
    meta->main_key = header.main_key;
    meta->pseudo_hash = pseudo_hash;

    for (const auto &dir : dirs)
    {
        auto dir_file_table_ptr = file_table_ptr + dir->file_table_offset;

        decrypt_strategy_2(
            dir_file_table_ptr,
            dir->file_table_size,
            header.main_key,
            pseudo_hash[2],
            0x7E);

        decrypt_strategy_1c(
            dir_file_table_ptr,
            dir->file_table_size,
            dir->file_table_main_key,
            pseudo_hash);

        io::MemoryStream file_table_stream(
            bstr(dir_file_table_ptr, dir->file_table_size));

        for (const auto i : algo::range(dir->file_count))
        {
            auto entry = std::make_unique<ArchiveEntryImpl>();
            const auto entry_offset = file_table_stream.tell();
            const auto entry_size = file_table_stream.read_u32_le();
            entry->offset = file_table_stream.read_u64_le() + data_start;
            entry->size = file_table_stream.read_u32_le();
            file_table_stream.skip(4);
            const auto tmp = file_table_stream.read_u32_le();
            entry->key
                = (header.main_key ^ (dir->file_table_main_key + tmp))
                + header.dir_count - 0x5C29E87B;
            const auto name = file_table_stream.read_to_zero();
            entry->path = algo::sjis_to_utf8(name).str();
            if (dir->path.name() != "root")
                entry->path = dir->path / entry->path;
            meta->entries.push_back(std::move(entry));
            file_table_stream.seek(entry_offset + entry_size);
        }
    }
    return std::move(meta);
}

std::unique_ptr<io::File> Cpz5ArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size);

    decrypt_strategy_3(
        data.get<u8>(),
        data.size(),
        meta->pseudo_hash[3],
        meta->main_key,
        meta->pseudo_hash,
        entry->key);

    return std::make_unique<io::File>(entry->path, data);
}

static auto dummy
    = fmt::register_fmt<Cpz5ArchiveDecoder>("purple-software/cpz5");
