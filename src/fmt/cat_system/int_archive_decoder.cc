#include "fmt/cat_system/int_archive_decoder.h"
#include "algo/crypt/blowfish.h"
#include "algo/crypt/mt.h"
#include "algo/range.h"
#include "algo/str.h"
#include "err.h"
#include "fmt/microsoft/exe_archive_decoder.h"
#include "io/file_system.h"

using namespace au;
using namespace au::fmt::cat_system;

static const bstr magic = "KIF\x00"_b;

namespace
{
    struct ResourceKeys final
    {
        bstr key_code;
        bstr v_code;
        bstr v_code2;
    };

    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        bstr file_key;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static bstr decrypt_name(const bstr &input, const u32 seed)
{
    static const std::string fwd =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    static const auto rev = algo::reverse(fwd);
    u32 key = algo::crypt::MersenneTwister::Classic(seed)->next_u32();
    u32 shift = static_cast<u8>((key >> 24) + (key >> 16) + (key >> 8) + key);
    bstr output(input);

    for (auto &p : output)
    {
        u32 index1 = 0;
        u32 index2 = shift;

        while (rev[index2 % rev.size()] != p)
        {
            if (rev[(shift + index1 + 1) % rev.size()] == p)
            {
                index1 += 1;
                break;
            }
            if (rev[(shift + index1 + 2) % rev.size()] == p)
            {
                index1 += 2;
                break;
            }
            if (rev[(shift + index1 + 3) % rev.size()] == p)
            {
                index1 += 3;
                break;
            }
            index1 += 4;
            index2 += 4;
            if (index1 > fwd.size())
                break;
        }

        if (index1 < fwd.size())
            p = fwd[index1];
        shift++;
    }
    return output;
}

static u32 get_table_seed(const bstr &input)
{
    static const u32 magic = 0x4C11DB7;
    u32 seed = 0xFFFFFFFF;
    for (const auto p : input)
    {
        seed ^= p << 24;
        for (const auto i : algo::range(8))
        {
            const auto bit = (seed & 0x80000000) != 0;
            seed <<= 1;
            if (bit)
                seed ^= magic;
        }
        seed = ~seed;
    }
    return seed;
}

static std::vector<io::path> find_executables(const io::path &archive_path)
{
    std::vector<io::path> paths;
    for (const auto path : io::directory_range(archive_path.parent()))
        if (path.has_extension("exe"))
            paths.push_back(path);
    return paths;
}

static ResourceKeys get_resource_keys(
    const Logger &logger, const std::vector<io::path> &executable_paths)
{
    if (executable_paths.empty())
        throw err::NotSupportedError("No executables found");

    ResourceKeys res_keys;
    for (const auto &path : executable_paths)
    {
        io::File file(path, io::FileMode::Read);
        fmt::microsoft::ExeArchiveDecoder exe_decoder;
        const auto meta = exe_decoder.read_meta(logger, file);
        for (const auto &entry : meta->entries)
        {
            const auto res_file = exe_decoder.read_file(
                logger, file, *meta, *entry);
            const auto res_content = res_file->stream.seek(0).read_to_eof();
            const auto res_name = algo::lower(entry->path.name());
            if (res_name.find("v_code2") != std::string::npos)
                res_keys.v_code2 = res_content;
            else if (res_name.find("v_code") != std::string::npos)
                res_keys.v_code = res_content;
            else if (res_name.find("key_code") != std::string::npos)
                res_keys.key_code = res_content;
        }
    }
    return res_keys;
}

static bstr get_game_id(const ResourceKeys &res_keys)
{
    bstr key = res_keys.key_code;
    for (auto &c : key)
        c ^= 0xCD;

    algo::crypt::Blowfish bf(key);
    return algo::trim_to_zero(bf.decrypt(res_keys.v_code2));
}

bool IntArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}
std::unique_ptr<fmt::ArchiveMeta> IntArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_u32_le();
    const auto name_size = 64;

    const auto executable_paths = find_executables(input_file.path);
    const auto resource_keys = get_resource_keys(logger, executable_paths);
    const auto game_id = get_game_id(resource_keys);

    bool encrypted = false;
    const auto table_seed = get_table_seed(game_id);

    auto meta = std::make_unique<ArchiveMetaImpl>();
    for (const auto i : algo::range(file_count))
    {
        const io::path entry_path = input_file.stream.read(name_size).str();
        const auto entry_offset = input_file.stream.read_u32_le();
        const auto entry_size = input_file.stream.read_u32_le();
        if (entry_path.name() == "__key__.dat")
        {
            auto mt = algo::crypt::MersenneTwister::Classic(entry_size);
            const auto tmp = mt->next_u32();
            meta->file_key = bstr(reinterpret_cast<const char*>(&tmp), 4);
            encrypted = true;
        }
    }

    input_file.stream.seek(8);
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();

        const auto name = input_file.stream.read(name_size);
        if (io::path(name.str()).name() == "__key__.dat")
        {
            input_file.stream.skip(8);
            continue;
        }

        if (encrypted)
        {
            entry->path = algo::trim_to_zero(
                decrypt_name(name, table_seed + i).str());

            algo::crypt::Blowfish bf(meta->file_key);
            bstr offset_and_size = input_file.stream.read(8);
            offset_and_size.get<u32>()[0] += i;
            offset_and_size = bf.decrypt(offset_and_size);
            entry->offset = offset_and_size.get<const u32>()[0];
            entry->size = offset_and_size.get<const u32>()[1];
        }
        else
        {
            entry->path = name.str();
            entry->offset = input_file.stream.read_u32_le();
            entry->size = input_file.stream.read_u32_le();
        }

        meta->entries.push_back(std::move(entry));
    }

    return std::move(meta);
}

std::unique_ptr<io::File> IntArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size);

    algo::crypt::Blowfish bf(meta->file_key);
    const auto crypt_size = (entry->size / bf.block_size()) * bf.block_size();
    data = bf.decrypt(data.substr(0, crypt_size)) + data.substr(crypt_size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> IntArchiveDecoder::get_linked_formats() const
{
    return {"cat-system/hg3"};
}

static auto dummy = fmt::register_fmt<IntArchiveDecoder>("cat-system/int");
