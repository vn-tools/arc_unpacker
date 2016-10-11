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

#include "dec/twilight_frontier/pak2_archive_decoder.h"
#include "algo/binary.h"
#include "algo/crypt/mt.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "dec/twilight_frontier/pak2_image_decoder.h"
#include "err.h"
#include "io/file_system.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::twilight_frontier;

static void decrypt(bstr &buffer, u32 mt_seed, u8 a, u8 b, u8 delta)
{
    auto mt = algo::crypt::MersenneTwister::Improved(mt_seed);
    for (const auto i : algo::range(buffer.size()))
    {
        buffer[i] ^= mt->next_u32();
        buffer[i] ^= a;
        a += b;
        b += delta;
    }
}

bool Pak2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    Logger dummy_logger;
    dummy_logger.mute();
    try
    {
        read_meta(dummy_logger, input_file);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::unique_ptr<dec::ArchiveMeta> Pak2ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto file_count = input_file.stream.read_le<u16>();
    if (!file_count && input_file.stream.size() != 6)
        throw err::RecognitionError();

    const auto table_size = input_file.stream.read_le<u32>();
    if (table_size > input_file.stream.left())
        throw err::RecognitionError();
    if (table_size > file_count * (4 + 4 + 256 + 1))
        throw err::RecognitionError();
    auto table_data = input_file.stream.read(table_size);
    decrypt(table_data, table_size + 6, 0xC5, 0x83, 0x53);
    io::MemoryByteStream table_stream(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->offset = table_stream.read_le<u32>();
        entry->size = table_stream.read_le<u32>();
        const auto name_size = table_stream.read<u8>();
        entry->path = algo::sjis_to_utf8(table_stream.read(name_size)).str();
        if (entry->offset + entry->size > input_file.stream.size())
            throw err::BadDataOffsetError();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Pak2ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = algo::unxor(
        input_file.stream.seek(entry->offset).read(entry->size),
        (entry->offset >> 1) | 0x23);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Pak2ArchiveDecoder::get_linked_formats() const
{
    return {"twilight-frontier/pak2-sfx", "twilight-frontier/pak2-gfx"};
}

static auto _ = dec::register_decoder<Pak2ArchiveDecoder>(
    "twilight-frontier/pak2");
