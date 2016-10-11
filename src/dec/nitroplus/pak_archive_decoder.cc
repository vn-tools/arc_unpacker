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

#include "dec/nitroplus/pak_archive_decoder.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::nitroplus;

static const bstr magic = "\x02\x00\x00\x00"_b;

namespace
{
    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        bool compressed;
    };
}

bool PakArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    Logger dummy_logger;
    dummy_logger.mute();
    return read_meta(dummy_logger, input_file)->entries.size() > 0;
}

std::unique_ptr<dec::ArchiveMeta> PakArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    const auto table_size_orig = input_file.stream.read_le<u32>();
    const auto table_size_comp = input_file.stream.read_le<u32>();
    input_file.stream.skip(0x104);

    io::MemoryByteStream table_stream(
        algo::pack::zlib_inflate(
            input_file.stream.read(table_size_comp)));

    auto meta = std::make_unique<ArchiveMeta>();
    const auto file_data_offset = input_file.stream.pos();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        const auto file_name_size = table_stream.read_le<u32>();
        entry->path = algo::sjis_to_utf8(
            table_stream.read(file_name_size)).str();
        entry->offset = table_stream.read_le<u32>() + file_data_offset;
        entry->size_orig = table_stream.read_le<u32>();
        table_stream.skip(4);
        entry->compressed = table_stream.read_le<u32>() > 0;
        entry->size_comp = table_stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> PakArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    const auto data = entry->compressed
        ? algo::pack::zlib_inflate(input_file.stream.read(entry->size_comp))
        : input_file.stream.read(entry->size_orig);
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<PakArchiveDecoder>("nitroplus/pak");
