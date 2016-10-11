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

#include "dec/leaf/pak2_group/pak2_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::leaf;

bool Pak2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("pak"))
        return false;
    const auto file_count = input_file.stream.seek(0x1C).read_le<u16>();
    if (file_count < 2)
        return false;
    const auto value1 = input_file.stream.seek(0x34).read_le<u32>();
    const auto value2 = input_file.stream.seek(0x3C).read_le<u32>();
    for (const auto i : algo::range(file_count))
    {
        if (input_file.stream.seek(0x34 + i * 0x20).read_le<u32>() != value1)
            return false;
        if (input_file.stream.seek(0x3C + i * 0x20).read_le<u32>() != value2)
            return false;
    }
    return true;
}

std::unique_ptr<dec::ArchiveMeta> Pak2ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    const auto file_count = input_file.stream.seek(0x1C).read_le<u16>();
    const auto data_offset = 0x20 + 0x20 * file_count;
    input_file.stream.seek(0x20);
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        input_file.stream.skip(2);
        auto name = input_file.stream.read(12);
        for (auto &c : name)
            c = (c << 4) | (c >> 4);
        entry->path = name.str(true);
        if (entry->path.str().empty())
            continue;
        input_file.stream.skip(2);
        entry->offset = input_file.stream.read_le<u32>() + data_offset;
        input_file.stream.skip(4);
        entry->size = input_file.stream.read_le<u32>();
        input_file.stream.skip(4);
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
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Pak2ArchiveDecoder::get_linked_formats() const
{
    return
    {
        "leaf/pak2-compressed-file",
        "leaf/pak2-audio",
        "leaf/pak2-image",
        "leaf/pak2-texture",
    };
}

static auto _ = dec::register_decoder<Pak2ArchiveDecoder>("leaf/pak2");
