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

#include "dec/chanchan/dat_archive_decoder.h"
#include <map>
#include "algo/locale.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::chanchan;

static std::unique_ptr<dec::ArchiveMeta> read_meta_v1(
    io::BaseByteStream &input_stream)
{
    const auto file_count = input_stream.seek(0).read_le<u32>();
    if (!file_count)
        return nullptr;
    auto meta = std::make_unique<dec::ArchiveMeta>();
    for (const size_t i : algo::range(file_count))
    {
        auto entry = std::make_unique<dec::CompressedArchiveEntry>();
        entry->path = algo::sjis_to_utf8(input_stream.read_to_zero(256)).str();
        entry->size_comp = entry->size_orig = input_stream.read_le<u32>();
        entry->offset = input_stream.pos();
        input_stream.skip(entry->size_comp);
        meta->entries.push_back(std::move(entry));
        if (i == file_count - 1 && input_stream.left())
            return nullptr;
    }
    return meta;
}

static std::unique_ptr<dec::ArchiveMeta> read_meta_v2(
    io::BaseByteStream &input_stream)
{
    input_stream.seek(0);
    const auto table_size = input_stream.read_le<u32>();
    const auto file_count = input_stream.read_le<u32>();
    if (!file_count || !table_size)
        return nullptr;
    auto meta = std::make_unique<dec::ArchiveMeta>();
    for (const size_t i : algo::range(file_count))
    {
        auto entry = std::make_unique<dec::CompressedArchiveEntry>();
        entry->path = algo::sjis_to_utf8(input_stream.read_to_zero(256)).str();
        entry->size_orig = entry->size_comp = input_stream.read_le<u32>();
        entry->offset = input_stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
        if (i == file_count - 1 && input_stream.pos() != table_size)
            return nullptr;
    }
    return meta;
}

static std::unique_ptr<dec::ArchiveMeta> read_meta_v3(
    io::BaseByteStream &input_stream)
{
    input_stream.seek(0);
    const auto file_count = input_stream.read_le<u32>();
    input_stream.skip(28);
    if (!file_count)
        return nullptr;
    auto meta = std::make_unique<dec::ArchiveMeta>();
    for (const size_t i : algo::range(file_count))
    {
        auto entry = std::make_unique<dec::CompressedArchiveEntry>();
        entry->path = algo::sjis_to_utf8(input_stream.read_to_zero(256)).str();
        const auto entry_size = input_stream.read_le<u32>();
        const auto entry_offset = input_stream.read_le<u32>();
        input_stream.skip(8);
        input_stream.peek(entry_offset, [&]()
        {
            input_stream.skip(16);
            entry->size_orig = input_stream.read_le<u32>();
            entry->size_comp = input_stream.read_le<u32>();
            input_stream.skip(12);
            entry->offset = input_stream.pos();
        });
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

static const std::map<int, decltype(&read_meta_v1)> meta_decoders
{
    {1, read_meta_v1},
    {2, read_meta_v2},
    {3, read_meta_v3},
};

static int tell_version(io::BaseByteStream &input_stream)
{
    for (const auto kv : meta_decoders)
    {
        try
        {
            if (kv.second(input_stream) != nullptr)
                return kv.first;
        }
        catch (...)
        {
        }
    }
    return -1;
}

bool DatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("dat"))
        return false;
    return tell_version(input_file.stream) != -1;
}

std::unique_ptr<dec::ArchiveMeta> DatArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto version = tell_version(input_file.stream);
    const auto meta_decoder = meta_decoders.at(version);
    return meta_decoder(input_file.stream);
}

std::unique_ptr<io::File> DatArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CompressedArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);
    if (entry->size_orig != entry->size_comp)
        data = algo::pack::lzss_decompress(data, entry->size_orig);
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<DatArchiveDecoder>("chanchan/dat");
