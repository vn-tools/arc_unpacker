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

#include "dec/bluearrowgarden/bin_archive_decoder.h"
#include "algo/format.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::bluearrowgarden;

namespace
{
    struct BinSegment final
    {
        u32 flags;
        size_t offset;
        size_t size_orig;
        size_t size_comp;
    };

    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        std::vector<BinSegment> segments;
    };
}

bool BinArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("bin"))
        return false;

    input_file.stream.seek(input_file.stream.size() - 4);
    const auto table_size_comp = input_file.stream.read_le<u32>();
    input_file.stream.seek(input_file.stream.size() - 8 - table_size_comp);
    return input_file.stream.read<u8>() == 'x'; // zlib header
}

std::unique_ptr<dec::ArchiveMeta> BinArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(input_file.stream.size() - 8);
    const auto table_size_orig = input_file.stream.read_le<u32>();
    const auto table_size_comp = input_file.stream.read_le<u32>();
    io::MemoryByteStream table_stream(algo::pack::zlib_inflate(
        input_file.stream
            .seek(input_file.stream.size() - 8 - table_size_comp)
            .read(table_size_comp)));

    const auto file_count = table_stream.read_le<u32>();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        const auto segment_count = table_stream.read_le<u32>();
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = table_stream.read_to_zero(44).str();
        for (const auto j : algo::range(segment_count))
        {
            BinSegment segment;
            segment.flags = table_stream.read_le<u32>(),
            segment.offset = table_stream.read_le<u32>();
            segment.size_comp = table_stream.read_le<u32>();
            segment.size_orig = table_stream.read_le<u32>();
            entry->segments.push_back(segment);
        }
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> BinArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    auto ret = std::make_unique<io::File>(entry->path, ""_b);
    for (const auto &segment : entry->segments)
    {
        ret->stream.write(
            algo::pack::zlib_inflate(
                input_file.stream
                    .seek(segment.offset)
                    .read(segment.size_comp)));
    }
    ret->stream.seek(0);
    ret->guess_extension();
    return ret;
}

std::vector<std::string> BinArchiveDecoder::get_linked_formats() const
{
    return {"bluearrowgarden/images"};
}

static auto _ = dec::register_decoder<BinArchiveDecoder>("bluearrowgarden/bin");
