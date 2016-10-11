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

#include "dec/twilight_frontier/pak1_archive_decoder.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::twilight_frontier;

static void decrypt(bstr &buffer, u8 a, u8 b, u8 delta)
{
    for (const auto i : algo::range(buffer.size()))
    {
        buffer[i] ^= a;
        a += b;
        b += delta;
    }
}

static std::unique_ptr<io::MemoryByteStream> read_raw_table(
    io::BaseByteStream &input_stream, size_t file_count)
{
    const auto table_size = file_count * 0x6C;
    if (table_size > input_stream.left())
        throw err::RecognitionError();
    if (table_size > file_count * (0x64 + 4 + 4))
        throw err::RecognitionError();
    auto buffer = input_stream.read(table_size);
    decrypt(buffer, 0x64, 0x64, 0x4D);
    return std::make_unique<io::MemoryByteStream>(buffer);
}

bool Pak1ArchiveDecoder::is_recognized_impl(io::File &input_file) const
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

std::unique_ptr<dec::ArchiveMeta> Pak1ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto file_count = input_file.stream.read_le<u16>();
    if (file_count == 0 && input_file.stream.size() != 6)
        throw err::RecognitionError();
    auto table_stream = read_raw_table(input_file.stream, file_count);
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = table_stream->read_to_zero(0x64).str();
        entry->size = table_stream->read_le<u32>();
        entry->offset = table_stream->read_le<u32>();
        if (entry->offset + entry->size > input_file.stream.size())
            throw err::BadDataOffsetError();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Pak1ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    auto output_file = std::make_unique<io::File>(entry->path, ""_b);
    auto data = input_file.stream.seek(entry->offset).read(entry->size);
    if (output_file->path.name() == "musicroom.dat")
    {
        decrypt(data, 0x5C, 0x5A, 0x3D);
        output_file->path.change_extension(".txt");
    }
    else if (output_file->path.has_extension("sce"))
    {
        decrypt(data, 0x63, 0x62, 0x42);
        output_file->path.change_extension(".txt");
    }
    else if (output_file->path.name() == "cardlist.dat")
    {
        decrypt(data, 0x60, 0x61, 0x41);
        output_file->path.change_extension(".txt");
    }
    output_file->stream.write(data);
    return output_file;
}

std::vector<std::string> Pak1ArchiveDecoder::get_linked_formats() const
{
    return {"twilight-frontier/pak1-sfx", "twilight-frontier/pak1-gfx"};
}

static auto _ = dec::register_decoder<Pak1ArchiveDecoder>(
    "twilight-frontier/pak1");
