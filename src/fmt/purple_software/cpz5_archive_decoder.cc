#include "fmt/purple_software/cpz5_archive_decoder.h"
#include "algo/binary.h"
#include "algo/crypt/md5.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"
#include "ptr.h"

using namespace au;
using namespace au::fmt::purple_software;

static const bstr magic = "CPZ5"_b;

namespace
{
    enum class HashKind : u8 { A, B };

    struct Plugin final
    {
        HashKind hash_kind;
        bstr secret;
        u32 strategy_2_3_mul;
        u32 entry_init_key;
        u8 entry_tail_key;
    };

    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        Plugin plugin;
        u32 main_key;
        std::array<u32, 4> hash;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        u32 key;
    };

    struct Header final
    {
        size_t dir_count;
        size_t dir_table_size;
        size_t file_table_size;
        std::array<u32, 4> md5_dwords;
        u32 main_key;
        size_t table_size;
        size_t data_start;
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

static std::shared_ptr<Plugin> get_v1_plugin()
{
    auto p = std::make_shared<Plugin>();
    p->hash_kind = HashKind::A;
    p->secret =
        "\x89\xF0\x90\xCD\x82\xB7\x82\xE9\x88\xAB\x82\xA2\x8E\x71\x82\xCD"
        "\x83\x8A\x83\x52\x82\xAA\x82\xA8\x8E\x64\x92\x75\x82\xAB\x82\xB5"
        "\x82\xBF\x82\xE1\x82\xA2\x82\xDC\x82\xB7\x81\x42\x8E\xF4\x82\xED"
        "\x82\xEA\x82\xBF\x82\xE1\x82\xA2\x82\xDC\x82\xB7\x82\xE6\x81\x60"
        "\x81\x41\x82\xC6\x82\xA2\x82\xA4\x82\xA9\x82\xE0\x82\xA4\x8E\xF4"
        "\x82\xC1\x82\xBF\x82\xE1\x82\xA2\x82\xDC\x82\xB5\x82\xBD\x81\xF4"_b;
    p->entry_init_key = 0x2547A39E;
    p->entry_tail_key = 0xBC;
    p->strategy_2_3_mul = 0x1A743125;
    return p;
}

static std::shared_ptr<Plugin> get_v2_plugin()
{
    auto p = std::make_shared<Plugin>();
    p->hash_kind = HashKind::B;
    p->secret =
        "\x89\xF0\x90\xCD\x82\xB7\x82\xE9\x88\xAB\x82\xA2\x8E\x71\x82\xCD"
        "\x83\x8A\x83\x52\x82\xAA\x82\xA8\x8E\x64\x92\x75\x82\xAB\x82\xB5"
        "\x82\xBF\x82\xE1\x82\xA2\x82\xDC\x82\xB7\x81\x42\x83\x81\x83\x62"
        "\x81\x49\x82\xA9\x82\xED\x82\xA2\x82\xA2\x82\xC6\x82\xA9\x8C\xBE"
        "\x82\xC1\x82\xC4\x82\xE0\x8B\x96\x82\xB5\x82\xC4\x82\xA0\x82\xB0"
        "\x82\xC8\x82\xA2\x82\xF1\x82\xBE\x82\xA9\x82\xE7\x82\x9F\x81\x49"_b;
    p->entry_init_key = 0x2547A39E;
    p->entry_tail_key = 0xCB;
    p->strategy_2_3_mul = 0x1A740235;
    return p;
}

static std::vector<std::shared_ptr<Plugin>> get_all_plugins()
{
    std::vector<std::shared_ptr<Plugin>> plugins;
    plugins.push_back(std::move(get_v1_plugin()));
    plugins.push_back(std::move(get_v2_plugin()));
    return plugins;
}

static void decrypt_strategy_1a(
    ptr<u8> target, const Plugin &plugin, const u32 key)
{
    std::array<u32, 24> table;
    const auto limit = std::min(table.size(), plugin.secret.size() / 4);
    for (const auto i : algo::range(limit))
        table[i] = plugin.secret.get<u32>()[i] - key;

    size_t shift = key;
    for (const auto i : algo::range(3))
        shift = (shift >> 8) ^ key;
    shift = ((shift ^ 0xFB) & 0x0F) + 7;

    size_t table_pos = 5;

    auto target_u32 = reinterpret_cast<u32*>(&target[0]);
    for (const auto i : algo::range(target.size() >> 2))
    {
        const auto tmp = (table[table_pos++] ^ *target_u32) + 0x784C5962;
        *target_u32++ = algo::rotr<u32>(tmp, shift) + 0x01010101;
        table_pos %= table.size();
    }

    auto target_u8 = reinterpret_cast<u8*>(target_u32);
    for (auto i : algo::range(target.size() & 3))
    {
        *target_u8
            = ((table[table_pos++] >> (4 * (3 - i))) ^ *target_u8) - 0x79;
        target_u8++;
        table_pos %= table.size();
    }
}

static void decrypt_strategy_1b(
    ptr<u8> target, const u32 key, const std::array<u32, 4> &hash)
{
    static const std::array<u32, 4> addends = {0x76A3BF29, 0, 0x10000000, 0};

    std::array<u32, 4> table;
    for (const auto i : algo::range(4))
        table[i] = hash[i] ^ (key + addends[i]);

    size_t table_pos = 0;

    auto target_u32 = reinterpret_cast<u32*>(&target[0]);
    u32 seed = 0x76548AEF;
    for (const auto i : algo::range(target.size() >> 2))
    {
        u32 tmp = (*target_u32 ^ table[table_pos++]) - 0x4A91C262;
        *target_u32++ = algo::rotl<u32>(tmp, 3) - seed;
        table_pos %= table.size();
        seed += 0x10FB562A;
    }

    auto target_u8 = reinterpret_cast<u8*>(target_u32);
    for (const auto i : algo::range(target.size() & 3))
    {
        *target_u8 = ((table[table_pos++] >> 6) ^ *target_u8) + 0x37;
        target_u8++;
        table_pos %= table.size();
    }
}

static void decrypt_strategy_1c(
    ptr<u8> target, const u32 key, const std::array<u32, 4> &hash)
{
    static const std::array<u32, 4> addends = {0, 0x112233, 0, 0x34258765};

    std::array<u32, 4> table;
    for (const auto i : algo::range(4))
        table[i] = hash[i] ^ (key + addends[i]);

    size_t table_pos = 0;

    auto target_u32 = reinterpret_cast<u32*>(&target[0]);
    u32 seed = 0x2A65CB4E;
    for (const auto i : algo::range(target.size() >> 2))
    {
        u32 tmp = (*target_u32 ^ table[table_pos++]) - seed;
        *target_u32++ = algo::rotl<u32>(tmp, 2) + 0x37A19E8B;
        table_pos %= table.size();
        seed -= 0x139FA9B;
    }

    auto target_u8 = reinterpret_cast<u8*>(target_u32);
    for (const auto i : algo::range(target.size() & 3))
    {
        *target_u8 = ((table[table_pos++] >> 4) ^ *target_u8) + 0x05;
        target_u8++;
        table_pos %= table.size();
    }
}

static std::array<u8, 256> get_table_for_decrypt_strategy_2_and_3(
    const Plugin &plugin, u32 key, const u32 seed)
{
    std::array<u8, 256> table;
    for (const auto i : algo::range(256))
        table[i] = i;
    for (const auto i : algo::range(256))
    {
        std::swap(table[(key >> 16) & 0xFF], table[key & 0xFF]);
        std::swap(table[(key >> 8) & 0xFF], table[(key >> 24) & 0xFF]);
        key = seed + algo::rotr<u32>(key, 2) * plugin.strategy_2_3_mul;
    }
    return table;
}

static void decrypt_strategy_2(
    ptr<u8> target,
    const Plugin &plugin,
    const u32 key,
    const u32 seed,
    const u8 permutation_xor)
{
    const auto table
        = get_table_for_decrypt_strategy_2_and_3(plugin, key, seed);
    for (const auto i : algo::range(target.size()))
        target[i] = table[target[i] ^ permutation_xor];
}

static void decrypt_strategy_3(
    ptr<u8> target,
    const Plugin &plugin,
    const u32 key,
    const u32 seed,
    const std::array<u32, 4> &hash,
    const u32 entry_key)
{
    const auto table
        = get_table_for_decrypt_strategy_2_and_3(plugin, key, seed);

    u32 yet_another_table[24];
    for (const auto i : algo::range(96))
    {
        reinterpret_cast<u8*>(yet_another_table)[i]
            = table[plugin.secret[i]] ^ (hash[1] >> 2);
    }

    for (const auto i : algo::range(24))
        yet_another_table[i] ^= entry_key;

    size_t yet_another_table_pos = 9;
    u32 yet_another_key = plugin.entry_init_key;

    u32 *target_u32 = reinterpret_cast<u32*>(&target[0]);
    for (const auto i : algo::range(target.size() >> 2))
    {
        const auto tmp1 = *target_u32;
        const auto tmp2 = yet_another_table[yet_another_table_pos & 0x0F] >> 1;
        const auto tmp3 = yet_another_table[(yet_another_key >> 6) & 0x0F];
        const auto final_tmp = tmp1 ^ tmp2 ^ tmp3;
        *target_u32 = hash[yet_another_key & 3] ^ (final_tmp - entry_key);

        yet_another_table_pos++;
        yet_another_key += *target_u32 + entry_key;
        target_u32++;
    }

    u8 *target_u8 = reinterpret_cast<u8*>(target_u32);
    for (const auto i : algo::range(target.size() & 3))
        target_u8[i] = table[target_u8[i] ^ plugin.entry_tail_key];
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
    header.table_size = header.dir_table_size + header.file_table_size;
    header.data_start = input_stream.tell() + header.table_size;
    return header;
}

static std::array<u32, 4> get_hash(
    const HashKind hash_kind,
    const std::array<u32, 4> &input_dwords)
{
    io::MemoryStream tmp_stream;
    for (const auto &dword : input_dwords)
        tmp_stream.write_u32_le(dword);

    std::array<u32, 4> iv;
    if (hash_kind == HashKind::A)
        iv = {0xC74A2B01, 0xE7C8AB8F, 0xD8BEDC4E, 0x7302A4C5};
    else if (hash_kind == HashKind::B)
        iv = {0x53FE9B2C, 0xF2C93EA8, 0xEE81BA59, 0xA2C8973E};
    else
        throw std::logic_error("Invalid hash kind");

    const auto hash_bytes = algo::crypt::md5(
        tmp_stream.seek(0).read_to_eof(), iv);

    tmp_stream.seek(0).write(hash_bytes).seek(0);
    std::array<u32, 4> hash_dwords;
    for (const auto i : algo::range(4))
        hash_dwords[i] = tmp_stream.read_u32_le();

    std::array<u32, 4> result;
    if (hash_kind == HashKind::A)
    {
        result[0] = hash_dwords[3];
        result[1] = hash_dwords[1];
        result[2] = hash_dwords[2];
        result[3] = hash_dwords[0];
    }
    else if (hash_kind == HashKind::B)
    {
        result[0] = hash_dwords[1] ^ 0x49875325;
        result[1] = hash_dwords[2] + 0x54F46D7D;
        result[2] = hash_dwords[3] ^ 0xAD7948B7;
        result[3] = hash_dwords[0] + 0x1D0638AD;
    }
    else
        throw std::logic_error("Invalid hash kind");

    return result;
}

static std::unique_ptr<ArchiveMetaImpl> read_table(
    const Plugin &plugin, const Header &header, const bstr &table_data)
{
    const auto hash = get_hash(plugin.hash_kind, header.md5_dwords);
    bstr table_data_copy(table_data);
    auto table_data_ptr = make_ptr(table_data_copy);
    auto dir_table_ptr = make_ptr(&table_data_copy[0], header.dir_table_size);
    auto file_table_ptr = make_ptr(
        &table_data_copy[header.dir_table_size], header.file_table_size);

    // whole index
    decrypt_strategy_1a(table_data_ptr, plugin, header.main_key ^ 0x3795B39A);

    // just directories
    decrypt_strategy_2(dir_table_ptr, plugin, header.main_key, hash[1], 0x3A);
    decrypt_strategy_1b(dir_table_ptr, header.main_key, hash);

    DirectoryInfo *prev_dir = nullptr;
    std::vector<std::unique_ptr<DirectoryInfo>> dirs;
    io::MemoryStream dir_table_stream(table_data_copy);
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
        prev_dir->file_table_size = header.table_size
            - header.dir_table_size
            - prev_dir->file_table_offset;
    }

    auto meta = std::make_unique<ArchiveMetaImpl>();
    meta->plugin = plugin;
    meta->main_key = header.main_key;
    meta->hash = hash;

    for (const auto &dir : dirs)
    {
        auto dir_file_table_ptr = make_ptr(
            file_table_ptr + dir->file_table_offset, dir->file_table_size);

        decrypt_strategy_2(
            dir_file_table_ptr, plugin, header.main_key, hash[2], 0x7E);

        decrypt_strategy_1c(
            dir_file_table_ptr, dir->file_table_main_key, hash);

        io::MemoryStream file_table_stream(
            bstr(&dir_file_table_ptr[0], dir_file_table_ptr.size()));

        for (const auto i : algo::range(dir->file_count))
        {
            auto entry = std::make_unique<ArchiveEntryImpl>();
            const auto entry_offset = file_table_stream.tell();
            const auto entry_size = file_table_stream.read_u32_le();
            entry->offset = file_table_stream.read_u64_le() + header.data_start;
            entry->size = file_table_stream.read_u32_le();
            file_table_stream.skip(4);
            const auto tmp = file_table_stream.read_u32_le();
            entry->key
                = (header.main_key ^ (dir->file_table_main_key + tmp))
                + header.dir_count - 0x5C29E87B;
            entry->path
                = algo::sjis_to_utf8(file_table_stream.read_to_zero()).str();
            if (dir->path.name() != "root")
                entry->path = dir->path / entry->path;
            meta->entries.push_back(std::move(entry));
            file_table_stream.seek(entry_offset + entry_size);
        }
    }
    return meta;
}

bool Cpz5ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Cpz5ArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    const auto header = read_header(input_file.stream);
    auto table_data = input_file.stream.read(header.table_size);
    auto plugins = get_all_plugins();
    for (const auto &plugin : plugins)
    {
        try
        {
            auto meta = read_table(*plugin, header, table_data);
            if (meta)
                return std::move(meta);
        }
        catch (std::exception &e)
        {
            continue;
        }
    }
    throw err::NotSupportedError("Unsupported encryption");
}

std::unique_ptr<io::File> Cpz5ArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size);

    decrypt_strategy_3(
        make_ptr(data),
        meta->plugin,
        meta->hash[3],
        meta->main_key,
        meta->hash,
        entry->key);

    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Cpz5ArchiveDecoder::get_linked_formats() const
{
    return {"purple-software/ps2", "purple-software/pb3"};
}

static auto dummy
    = fmt::register_fmt<Cpz5ArchiveDecoder>("purple-software/cpz5");
