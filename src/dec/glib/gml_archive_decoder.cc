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

#include "dec/glib/gml_archive_decoder.h"
#include "algo/range.h"
#include "dec/glib/custom_lzss.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::glib;

static const bstr magic = "GML_ARC\x00"_b;

namespace
{
    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        bstr prefix;
    };

    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        bstr permutation;
    };
}

bool GmlArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> GmlArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_data_start = input_file.stream.read_le<u32>();
    const auto table_size_orig = input_file.stream.read_le<u32>();
    const auto table_size_comp = input_file.stream.read_le<u32>();
    auto table_data = input_file.stream.read(table_size_comp);
    for (const auto i : algo::range(table_data.size()))
        table_data[i] ^= 0xFF;
    table_data = custom_lzss_decompress(table_data, table_size_orig);
    io::MemoryByteStream table_stream(table_data);

    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->permutation = table_stream.read(0x100);

    const auto file_count = table_stream.read_le<u32>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = table_stream.read(table_stream.read_le<u32>()).str();
        entry->offset = table_stream.read_le<u32>() + file_data_start;
        entry->size = table_stream.read_le<u32>();
        entry->prefix = table_stream.read(4);
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> GmlArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);

    input_file.stream.seek(entry->offset);
    input_file.stream.skip(entry->prefix.size());
    auto suffix = input_file.stream.read(entry->size - entry->prefix.size());
    for (const auto i : algo::range(suffix.size()))
        suffix[i] = meta->permutation[suffix.get<u8>()[i]];

    auto output_file = std::make_unique<io::File>();
    output_file->path = entry->path;
    output_file->stream.write(entry->prefix);
    output_file->stream.write(suffix);
    return output_file;
}

std::vector<std::string> GmlArchiveDecoder::get_linked_formats() const
{
    return {"glib/pgx", "vorbis/wav"};
}

static auto _ = dec::register_decoder<GmlArchiveDecoder>("glib/gml");
