// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "dec/cat_system/int_archive_decoder.h"
#include "algo/crypt/blowfish.h"
#include "algo/crypt/mt.h"
#include "algo/range.h"
#include "algo/str.h"
#include "dec/microsoft/exe_archive_decoder.h"
#include "err.h"
#include "io/file_system.h"

using namespace au;
using namespace au::dec::cat_system;

static const bstr magic = "KIF\x00"_b;

namespace
{
    struct ResourceKeys final
    {
        bstr key_code;
        bstr v_code;
        bstr v_code2;
    };

    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        bstr file_key;
        bool encrypted;
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
        const auto exe_decoder = dec::microsoft::ExeArchiveDecoder();
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
std::unique_ptr<dec::ArchiveMeta> IntArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    const auto name_size = 64;

    const auto executable_paths = find_executables(input_file.path);
    const auto resource_keys = get_resource_keys(logger, executable_paths);
    const auto game_id = get_game_id(resource_keys);

    const auto table_seed = get_table_seed(game_id);

    auto meta = std::make_unique<CustomArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        const io::path entry_path = input_file.stream.read(name_size).str();
        const auto entry_offset = input_file.stream.read_le<u32>();
        const auto entry_size = input_file.stream.read_le<u32>();
        if (entry_path.name() == "__key__.dat")
        {
            auto mt = algo::crypt::MersenneTwister::Classic(entry_size);
            const auto tmp = mt->next_u32();
            meta->file_key = bstr(reinterpret_cast<const char*>(&tmp), 4);
            meta->encrypted = true;
        }
    }

    input_file.stream.seek(8);
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();

        const auto name = input_file.stream.read(name_size);
        if (io::path(name.str()).name() == "__key__.dat")
        {
            input_file.stream.skip(8);
            continue;
        }

        if (meta->encrypted)
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
            entry->offset = input_file.stream.read_le<u32>();
            entry->size = input_file.stream.read_le<u32>();
        }

        meta->entries.push_back(std::move(entry));
    }

    return std::move(meta);
}

std::unique_ptr<io::File> IntArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size);
    if (meta->encrypted)
    {
        const algo::crypt::Blowfish bf(meta->file_key);
        bf.decrypt_in_place(data);
    }
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> IntArchiveDecoder::get_linked_formats() const
{
    return {"cat-system/hg3"};
}

static auto _ = dec::register_decoder<IntArchiveDecoder>("cat-system/int");
