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

#include "dec/tanuki_soft/tac_archive_decoder.h"
#include "algo/crypt/blowfish.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::tanuki_soft;

namespace
{
    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        u64 hash;
        bool compressed;
    };

    struct Directory final
    {
        u16 hash;
        u16 entry_count;
        u32 start_index;
    };

    enum Version
    {
        Unknown,
        Version100,
        Version110,
    };
}

static const bstr magic_100 = "TArc1.00\x00\x00\x00\x00"_b;
static const bstr magic_110 = "TArc1.10\x00\x00\x00\x00"_b;

static bstr decrypt(const bstr &input, size_t size, const bstr &key)
{
    algo::crypt::Blowfish bf(key);
    const auto left = (size / bf.block_size()) * bf.block_size();
    return bf.decrypt(input.substr(0, left)) + input.substr(left);
}

static Version read_version(io::BaseByteStream &input_stream)
{
    if (input_stream.seek(0).read(magic_100.size()) == magic_100)
        return Version::Version100;
    input_stream.seek(0);
    if (input_stream.seek(0).read(magic_110.size()) == magic_110)
        return Version::Version110;
    return Version::Unknown;
}

bool TacArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return read_version(input_file.stream) != Version::Unknown;
}

std::unique_ptr<dec::ArchiveMeta> TacArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto version = read_version(input_file.stream);
    input_file.stream.skip(8);
    const auto entry_count = input_file.stream.read_le<u32>();
    const auto dir_count = input_file.stream.read_le<u32>();
    const auto table_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);
    if (version == Version::Version110)
        input_file.stream.skip(8);
    const auto file_data_start = input_file.stream.pos() + table_size;

    auto table_data = input_file.stream.read(table_size);
    table_data = decrypt(table_data, table_size, "TLibArchiveData"_b);
    table_data = algo::pack::zlib_inflate(table_data);
    io::MemoryByteStream table_stream(table_data);

    std::vector<std::unique_ptr<Directory>> dirs;
    for (const auto i : algo::range(dir_count))
    {
        auto dir = std::make_unique<Directory>();
        dir->hash = table_stream.read_le<u16>();
        dir->entry_count = table_stream.read_le<u16>();
        dir->start_index = table_stream.read_le<u32>();
        dirs.push_back(std::move(dir));
    }

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(entry_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->hash = table_stream.read_le<u64>();
        entry->compressed = table_stream.read_le<u32>() != 0;
        entry->size_orig = table_stream.read_le<u32>();
        entry->offset = table_stream.read_le<u32>() + file_data_start;
        entry->size_comp = table_stream.read_le<u32>();
        entry->path = algo::format("%05d.dat", i);
        meta->entries.push_back(std::move(entry));
    }

    for (auto &dir : dirs)
    {
        for (const auto i : algo::range(dir->entry_count))
        {
            if (i + dir->start_index >= meta->entries.size())
                throw err::CorruptDataError("Corrupt file table");
            auto entry = static_cast<CustomArchiveEntry*>(
                meta->entries[dir->start_index + i].get());
            entry->hash = (entry->hash << 16) | dir->hash;
        }
    }

    return meta;
}

std::unique_ptr<io::File> TacArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    auto data = input_file.stream
        .seek(entry->offset)
        .read(entry->size_comp);
    if (entry->compressed)
        data = algo::pack::zlib_inflate(data);

    if (!entry->compressed)
    {
        const auto key = algo::format("%llu_tlib_secure_", entry->hash);
        auto bytes_to_decrypt = std::min<size_t>(10240, data.size());

        {
            const auto header = decrypt(
                data.substr(0, algo::crypt::Blowfish::block_size()),
                algo::crypt::Blowfish::block_size(),
                key).substr(0, 4);
            if (header == "RIFF"_b || header == "TArc"_b)
                bytes_to_decrypt = data.size();
        }

        data = decrypt(data, bytes_to_decrypt, key);
    }

    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> TacArchiveDecoder::get_linked_formats() const
{
    return {"tanuki/tac"};
}

static auto _ = dec::register_decoder<TacArchiveDecoder>("tanuki/tac");
