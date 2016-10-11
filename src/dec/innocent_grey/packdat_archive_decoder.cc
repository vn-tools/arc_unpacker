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

#include "dec/innocent_grey/packdat_archive_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::innocent_grey;

static const bstr magic = "PACKDAT\x2E"_b;

namespace
{
    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        bool encrypted;
    };
}

bool PackdatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> PackdatArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = input_file.stream.read_to_zero(32).str();
        entry->offset = input_file.stream.read_le<u32>();
        entry->encrypted = (input_file.stream.read_le<u32>() & 0x10000) != 0;
        entry->size_orig = input_file.stream.read_le<u32>();
        entry->size_comp = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> PackdatArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size_comp);
    if (entry->size_comp != entry->size_orig)
        throw err::NotSupportedError("Compressed files are not supported");
    if (entry->encrypted)
    {
        auto data_ptr = data.get<u32>();
        const auto data_end = data.end<const u32>();
        const auto size = data.size() >> 2;
        u32 key = (size << ((size & 7) + 8)) ^ size;
        while (data_ptr < data_end)
        {
            const auto tmp = *data_ptr ^ key;
            const auto rot = tmp % 24;
            *data_ptr++ = tmp;
            key = (key << rot) | (key >> (32 - rot));
        }
    }
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<PackdatArchiveDecoder>(
    "innocent-grey/packdat");
