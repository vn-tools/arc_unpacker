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

#include "dec/abstraction/wad_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::abstraction;

static const bstr magic = "AGAR"_b;

static const std::string read_string(io::BaseByteStream &input_stream)
{
    const auto name_size = input_stream.read_le<u32>();
    return algo::utf8_to_sjis(input_stream.read(name_size)).str();
}

bool WadArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> WadArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto version_major = input_file.stream.read_le<u32>();
    const auto version_minor = input_file.stream.read_le<u32>();
    const auto extra_header_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(extra_header_size);

    auto meta = std::make_unique<ArchiveMeta>();

    const auto file_count = input_file.stream.read_le<u32>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = read_string(input_file.stream);
        entry->size = input_file.stream.read_le<u64>();
        entry->offset = input_file.stream.read_le<u64>();
        meta->entries.push_back(std::move(entry));
    }

    const auto dir_count = input_file.stream.read_le<u32>();
    for (const auto i : algo::range(dir_count))
    {
        const auto dir_name = read_string(input_file.stream);
        const auto dir_entry_count = input_file.stream.read_le<u32>();
        for (const auto j : algo::range(dir_entry_count))
        {
            const auto dir_entry_name = read_string(input_file.stream);
            const auto dir_entry_type = input_file.stream.read<u8>();
        }
    }

    const auto base_offset = input_file.stream.pos();
    for (auto &entry : meta->entries)
        static_cast<PlainArchiveEntry*>(entry.get())->offset += base_offset;

    return meta;
}

std::unique_ptr<io::File> WadArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> WadArchiveDecoder::get_linked_formats() const
{
    return {"truevision/tga"};
}

static auto _ = dec::register_decoder<WadArchiveDecoder>("abstraction/wad");
