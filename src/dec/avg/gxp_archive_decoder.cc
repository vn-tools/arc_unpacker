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

#include "dec/avg/gxp_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::avg;

static const bstr magic = "GXP\x00"_b;

static bstr decrypt(const bstr &input)
{
    static const auto key =
        "\x40\x21\x28\x38\xA6\x6E\x43\xA5"
        "\x40\x21\x28\x38\xA6\x43\xA5\x64"
        "\x3E\x65\x24\x20\x46\x6E\x74"_b;

    bstr output(input);
    for (const auto i : algo::range(output.size()))
        output[i] ^= key[i % key.size()] ^ i;
    return output;
}

bool GxpArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> GxpArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(24);
    const auto file_count = input_file.stream.read_le<u32>();
    const auto table_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(8);
    const auto data_offset = input_file.stream.read_le<u64>();
    io::MemoryByteStream table_stream(input_file.stream.read(table_size));

    auto meta = std::make_unique<dec::ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry_stream = std::make_unique<io::MemoryByteStream>(
            decrypt(table_stream.read(4)));
        const auto entry_size = entry_stream->read_le<u32>();
        entry_stream = std::make_unique<io::MemoryByteStream>(
            decrypt(table_stream.skip(-4).read(entry_size)));
        entry_stream->skip(4);

        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->size = entry_stream->read_le<u32>();
        entry_stream->skip(4);
        const auto name_size = entry_stream->read_le<u32>();
        entry_stream->skip(8);
        entry->offset = data_offset + entry_stream->read_le<u64>();
        entry->path
            = algo::utf16_to_utf8(entry_stream->read(name_size * 2)).str();
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> GxpArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, decrypt(data));
}

static auto _ = dec::register_decoder<GxpArchiveDecoder>("avg/gxp");
