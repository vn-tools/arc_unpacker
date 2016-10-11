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

#include "dec/adv/dat_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::adv;

static const bstr magic = "ARCHIVE\x00"_b;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        bstr game_key;
        bstr arc_key;
        size_t file_count;
        uoff_t header_offset;
        uoff_t table_offset;
    };
}

static std::unique_ptr<CustomArchiveMeta> prepare_meta(io::File &input_file)
{
    auto meta = std::make_unique<CustomArchiveMeta>();
    input_file.stream.seek(53); // suspicious: varies for other games?
    meta->arc_key = input_file.stream.read(8);

    input_file.stream.skip(20);
    meta->header_offset = input_file.stream.pos();
    bstr header_data = input_file.stream.read(0x24);

    // recover game key
    meta->game_key.resize(8);
    for (const auto i : algo::range(8))
        meta->game_key[i] = meta->arc_key[i] ^ header_data[i] ^ magic[i];

    for (const auto i : algo::range(8))
        meta->arc_key[i] ^= meta->game_key[i];

    for (const auto i : algo::range(0x24))
        header_data[i] ^= meta->arc_key[i % 8];

    meta->table_offset = header_data.get<const u32>()[4];
    meta->file_count = header_data.get<const u32>()[8];
    return meta;
}

bool DatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    const auto meta = prepare_meta(input_file);
    return meta->header_offset + meta->table_offset + 0x114 * meta->file_count
        == input_file.stream.size();
}

std::unique_ptr<dec::ArchiveMeta> DatArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = prepare_meta(input_file);

    input_file.stream.seek(meta->header_offset + meta->table_offset);
    auto table_data = input_file.stream.read(0x114 * meta->file_count);
    auto key_idx = meta->table_offset;
    for (const auto i : algo::range(table_data.size()))
        table_data[i] ^= meta->arc_key[(key_idx++) % meta->arc_key.size()];
    io::MemoryByteStream table_stream(table_data);

    for (const auto i : algo::range(meta->file_count))
    {
        table_stream.seek(0x114 * i);
        table_stream.skip(1);
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = algo::sjis_to_utf8(table_stream.read_to_zero()).str();
        table_stream.seek(0x114 * i + 0x108);
        entry->offset = table_stream.read_le<u32>() + meta->header_offset;
        entry->size = table_stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> DatArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto key_idx = entry->offset - meta->header_offset;
    for (const auto i : algo::range(data.size()))
        data[i] ^= meta->arc_key[key_idx++ % meta->arc_key.size()];
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<DatArchiveDecoder>("adv/dat");
