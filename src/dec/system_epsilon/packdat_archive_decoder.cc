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

#include "dec/system_epsilon/packdat_archive_decoder.h"
#include "algo/binary.h"
#include "algo/ptr.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::system_epsilon;

static const auto magic = "PACKDAT."_b;

namespace
{
    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        u32 flags;
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
        entry->flags = input_file.stream.read_le<u32>();
        entry->size_orig = input_file.stream.read_le<u32>();
        entry->size_comp = input_file.stream.read_le<u32>(); // or vice versa
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> PackdatArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    if (entry->size_orig != entry->size_comp)
        throw err::NotSupportedError("Compressed entries are not supported");
    auto data = input_file.stream.seek(entry->offset).read(entry->size_orig);
    if (entry->flags & 0x10000)
    {
        u32 key = data.size() >> 2;
        key = (key << ((key & 7) + 8)) ^ key;
        auto data_ptr = algo::make_ptr(data.get<u32>(), data.size() / 4);
        while (data_ptr.left())
        {
            *data_ptr ^= key;
            key = algo::rotl<u32>(key, *data_ptr++ % 24);
        }
    }
    return std::make_unique<io::File>(entry->path, data);
}

static auto _
    = dec::register_decoder<PackdatArchiveDecoder>("system-epsilon/packdat");
