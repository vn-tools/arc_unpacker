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

#include "dec/gs/dat_archive_decoder.h"
#include "algo/format.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::gs;

static const bstr magic = "GsSYMBOL5BINDATA"_b;

bool DatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> DatArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0xA8);
    const auto file_count = input_file.stream.read_le<u32>();
    input_file.stream.skip(12);
    const auto table_offset = input_file.stream.read_le<u32>();
    const auto table_size_comp = input_file.stream.read_le<u32>();
    const auto key = input_file.stream.read_le<u32>();
    const auto table_size_orig = input_file.stream.read_le<u32>();
    const auto data_offset = input_file.stream.read_le<u32>();

    input_file.stream.seek(table_offset);
    auto table_data = input_file.stream.read(table_size_comp);
    for (const auto i : algo::range(table_data.size()))
        table_data[i] ^= i & key;
    table_data = algo::pack::lzss_decompress(table_data, table_size_orig);
    io::MemoryByteStream table_stream(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        table_stream.seek(i * 0x18);
        auto entry = std::make_unique<CompressedArchiveEntry>();
        entry->path = algo::format("%05d.dat", i);
        entry->offset = table_stream.read_le<u32>() + data_offset;
        entry->size_comp = table_stream.read_le<u32>();
        entry->size_orig = table_stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> DatArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CompressedArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);
    data = algo::pack::lzss_decompress(data, entry->size_orig);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

static auto _ = dec::register_decoder<DatArchiveDecoder>("gs/dat");
