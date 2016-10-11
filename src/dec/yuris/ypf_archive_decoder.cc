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

#include "dec/yuris/ypf_archive_decoder.h"
#include <set>
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::yuris;

static const bstr magic = "YPF\x00"_b;

namespace
{
    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        u8 type;
        bool compressed;
    };
}

static std::vector<u8> create_swap_table(
    const std::vector<std::pair<u8, u8>> &swaps)
{
    std::vector<u8> ret;
    for (const auto i : algo::range(256))
        ret.push_back(i);
    for (auto &pair : swaps)
    {
        ret[pair.first] = pair.second;
        ret[pair.second] = pair.first;
    }
    return ret;
}

static const std::vector<std::vector<u8>> name_size_tables = {
    create_swap_table({
        {0x03, 0x48}, {0x06, 0x35}, {0x0C, 0x10}, {0x11, 0x19},
        {0x1C, 0x1E}, {0x09, 0x0B}, {0x0D, 0x13}, {0x15, 0x1B},
        {0x20, 0x23}, {0x26, 0x29}, {0x2C, 0x2F}, {0x2E, 0x32},
    }),

    create_swap_table({
        {0x0C, 0x10}, {0x11, 0x19}, {0x1C, 0x1E}, {0x09, 0x0B},
        {0x0D, 0x13}, {0x15, 0x1B}, {0x20, 0x23}, {0x26, 0x29},
        {0x2C, 0x2F}, {0x2E, 0x32},
    }),

    create_swap_table({
        {0x09, 0x0B}, {0x0D, 0x13}, {0x15, 0x1B}, {0x20, 0x23},
        {0x26, 0x29}, {0x2C, 0x2F}, {0x2E, 0x32},
    }),

    create_swap_table({
        {0x03, 0x48}, {0x06, 0x35}, {0x09, 0x0B}, {0x0C, 0x10},
        {0x0D, 0x13}, {0x11, 0x19}, {0x15, 0x1B}, {0x1C, 0x1E},
        {0x20, 0x23}, {0x26, 0x29}, {0x2C, 0x2F}, {0x2E, 0x32},
    }),
};

static bstr read_raw_name(
    io::BaseByteStream &input_stream,
    const std::vector<u8> &name_size_table)
{
    auto name_size = input_stream.read<u8>() ^ 0xFF;
    return input_stream.read(name_size_table.at(name_size));
}

static size_t guess_extra_header_size(const int version)
{
    if (version == 0xDE)
        return 12;
    if (version >= 0x1D9)
        return 8;
    return 4;
}

static std::vector<u8> guess_name_size_table(
    io::BaseByteStream &input_stream,
    const size_t file_count,
    const int version)
{
    const auto extra_header_size = guess_extra_header_size(version);
    for (const auto &name_size_table : name_size_tables)
    {
        input_stream.seek(0);
        try
        {
            for (const auto i : algo::range(file_count))
            {
                input_stream.skip(4);
                const auto name = read_raw_name(input_stream, name_size_table);
                input_stream.skip(14);
                input_stream.skip(extra_header_size);
            }
            input_stream.seek(0);
            return name_size_table;
        }
        catch (...)
        {
        }
    }
    throw err::NotSupportedError("Failed to guess name size permutation table");
}

static u8 guess_key(const std::vector<bstr> &names)
{
    static const std::set<std::string> good_extensions
        = {"bmp", "png", "ogg", "wav", "txt", "ybn"};

    for (const auto &name : names)
    {
        if (name.size() < 4)
            continue;
        const auto key = name.at(name.size() - 4) ^ '.';
        const auto decoded_name = algo::unxor(name, key);
        const auto possible_extension = decoded_name.substr(-3).str();
        if (good_extensions.find(possible_extension) != good_extensions.end())
            return key;
    }
    throw err::NotSupportedError("Failed to guess the key");
}

bool YpfArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> YpfArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(4);
    const auto version = input_file.stream.read_le<u32>();
    const auto file_count = input_file.stream.read_le<u32>();
    const auto table_size = input_file.stream.read_le<u32>();

    io::MemoryByteStream table_stream(
        input_file.stream.seek(0x20).read(table_size));

    const auto name_size_table = guess_name_size_table(
        table_stream, file_count, version);
    const auto extra_header_size = guess_extra_header_size(version);

    std::vector<bstr> names;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        table_stream.skip(4);

        const auto name = read_raw_name(table_stream, name_size_table);
        names.push_back(name);

        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->type = table_stream.read<u8>();
        entry->compressed = table_stream.read<u8>() == 1;
        entry->size_orig = table_stream.read_le<u32>();
        entry->size_comp = table_stream.read_le<u32>();
        entry->offset = table_stream.read_le<u32>();
        table_stream.skip(extra_header_size);
        meta->entries.push_back(std::move(entry));
    }

    const auto key = guess_key(names);
    for (const auto i : algo::range(file_count))
    {
        meta->entries[i]->path = algo::sjis_to_utf8(
            algo::unxor(names[i], key)).str();
    }

    return meta;
}

std::unique_ptr<io::File> YpfArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);
    if (entry->compressed)
        data = algo::pack::zlib_inflate(data);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> YpfArchiveDecoder::get_linked_formats() const
{
    return {"yuris/ycg"};
}

static auto _ = dec::register_decoder<YpfArchiveDecoder>("yuris/ypf");
