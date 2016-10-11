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

#include "dec/nekopack/nekopack4_archive_decoder.h"
#include <map>
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "dec/microsoft/bmp_image_decoder.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::nekopack;

static const bstr magic = "NEKOPACK4A"_b;

namespace
{
    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        u32 offset;
        u32 size_comp;
    };
}

bool Nekopack4ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Nekopack4ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto table_size = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<ArchiveMeta>();
    while (true)
    {
        const auto name_size = input_file.stream.read_le<u32>();
        if (!name_size)
            break;

        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = input_file.stream.read_to_zero(name_size).str();
        u32 key = 0;
        for (const u8 &c : entry->path.str())
            key += c;
        entry->offset = input_file.stream.read_le<u32>() ^ key;
        entry->size_comp = input_file.stream.read_le<u32>() ^ key;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Nekopack4ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    auto data = input_file.stream
        .seek(entry->offset)
        .read(entry->size_comp - 4);
    const auto size_orig = input_file.stream.read_le<u32>();

    u8 key = (entry->size_comp >> 3) + 0x22;
    for (auto &c : data)
    {
        c ^= key;
        key <<= 3;
        if (!key)
            break;
    }

    data = algo::pack::zlib_inflate(data);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Nekopack4ArchiveDecoder::get_linked_formats() const
{
    return {"nekopack/masked-bmp"};
}

static auto _ = dec::register_decoder<Nekopack4ArchiveDecoder>(
    "nekopack/nekopack4");
