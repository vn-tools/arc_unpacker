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

#include "dec/pajamas/gamedat_archive_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::pajamas;

static const auto magic = "GAMEDAT PAC"_b;

static const auto get_version(io::BaseByteStream &input_stream)
{
    const auto byte = input_stream.seek(magic.size()).read<u8>();
    if (byte == 'K')
        return 1;
    if (byte == '2')
        return 2;
    throw err::RecognitionError();
}

bool GamedatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> GamedatArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto version = get_version(input_file.stream);
    const auto file_count = input_file.stream.read_le<u32>();
    const auto name_size = version == 1 ? 16 : 32;

    const auto names_offset = input_file.stream.pos();
    const auto table_offset = names_offset + name_size * file_count;
    const auto data_offset = table_offset + 8 * file_count;

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = input_file.stream
            .seek(names_offset + i * name_size)
            .read_to_zero(name_size)
            .str();

        input_file.stream.seek(table_offset + i * 8);
        entry->offset = input_file.stream.read_le<u32>() + data_offset;
        entry->size = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> GamedatArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size);

    if (data.substr(0, 5) == "\x95\x6B\x3C\x9D\x63"_b)
    {
        u8 key = 0xC5;
        for (auto &c : data)
        {
            c ^= key;
            key += 0x5C;
        }
    }

    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> GamedatArchiveDecoder::get_linked_formats() const
{
    return {"pajamas/ep"};
}

static auto _
    = dec::register_decoder<GamedatArchiveDecoder>("pajamas/gamedat");
