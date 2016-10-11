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

#include "dec/libido/arc_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::libido;

bool ArcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    const auto file_count = input_file.stream.read_le<u32>();
    if (file_count)
    {
        input_file.stream.skip((file_count - 1) * 32 + 24);
        const auto last_file_size_comp = input_file.stream.read_le<u32>();
        const auto last_file_offset = input_file.stream.read_le<u32>();
        input_file.stream.seek(last_file_size_comp + last_file_offset);
    }
    else
        input_file.stream.skip(1);
    return input_file.stream.left() == 0;
}

std::unique_ptr<dec::ArchiveMeta> ArcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    const auto file_count = input_file.stream.read_le<u32>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CompressedArchiveEntry>();
        auto tmp = input_file.stream.read(20);
        for (const auto i : algo::range(tmp.size()))
            tmp[i] ^= tmp[tmp.size() - 1];
        entry->path = tmp.str(true);
        entry->size_orig = input_file.stream.read_le<u32>();
        entry->size_comp = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> ArcArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CompressedArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);
    data = algo::pack::lzss_decompress(data, entry->size_orig);
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<ArcArchiveDecoder>("libido/arc");
