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

#include "dec/unity/assets_archive_decoder.h"
#include "dec/unity/assets_archive_decoder/meta.h"
#include "err.h"

using namespace au;
using namespace au::dec::unity;

namespace
{
    struct Header final
    {
        uoff_t metadata_size;
        uoff_t file_size;
        u32 version;
        uoff_t data_offset;
        u8 endianness;
    };
}

static Header read_header(CustomStream &input_stream)
{
    Header header;
    header.metadata_size = input_stream.read<u32>();
    header.file_size = input_stream.read<u32>();
    header.version = input_stream.read<u32>();
    header.data_offset = input_stream.read<u32>();
    if (header.version >= 9)
    {
        header.endianness = input_stream.read<u8>();
        input_stream.skip(3);
    }
    return header;
}

bool AssetsArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    // TODO: support .split0 files
    return input_file.path.has_extension("assets");
}

std::unique_ptr<dec::ArchiveMeta> AssetsArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    CustomStream custom_stream(input_file.stream);
    const auto header = read_header(custom_stream);

    if (header.version < 9)
        throw err::NotSupportedError("Object data order not implemented");

    if (header.version > 5)
        custom_stream.set_endianness(algo::Endianness::LittleEndian);

    Meta assets_meta(custom_stream, header.version);
    const auto &object_info_map = *assets_meta.object_info_table;
    const auto &type_tree_map = *assets_meta.type_tree;

    auto meta = std::make_unique<ArchiveMeta>();

    for (const auto &object_info_kv : object_info_map)
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        const auto &object_info = object_info_kv.second;
        entry->offset = header.data_offset + object_info->offset;
        entry->size = object_info->size;
        // const auto &type = type_tree_map.at(object_info->type_id);
        meta->entries.push_back(std::move(entry));
    }

    return meta;
}

std::unique_ptr<io::File> AssetsArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<AssetsArchiveDecoder>("unity/assets");
