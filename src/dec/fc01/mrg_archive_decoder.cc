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

#include "dec/fc01/mrg_archive_decoder.h"
#include "algo/range.h"
#include "dec/fc01/common/custom_lzss.h"
#include "dec/fc01/common/mrg_decryptor.h"
#include "dec/fc01/common/util.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::fc01;

static const bstr magic = "MRG\x00"_b;

namespace
{
    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        u8 filter;
    };
}

static u8 guess_key(const bstr &table_data, size_t file_size)
{
    u8 tmp = common::rol8(table_data.get<u8>()[table_data.size() - 1], 1);
    u8 key = tmp ^ (file_size >> 24);
    u32 pos = 1;
    u32 last_offset = tmp ^ key;
    for (auto i = table_data.size() - 2; i >= table_data.size() - 4; --i)
    {
        key -= ++pos;
        tmp = common::rol8(table_data.get<u8>()[i], 1);
        last_offset = (last_offset << 8) | (tmp ^ key);
    }
    if (last_offset != file_size)
        throw err::NotSupportedError("Failed to guess the key");
    while (pos++ < table_data.size())
        key -= pos;
    return key;
}

bool MrgArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> MrgArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 4);
    const auto data_offset = input_file.stream.read_le<u32>();
    const auto table_size = data_offset - 12 - magic.size();
    const auto file_count = input_file.stream.read_le<u32>();

    auto table_data = input_file.stream.read(table_size);
    auto key = guess_key(table_data, input_file.stream.size());
    for (const auto i : algo::range(table_data.size()))
    {
        table_data[i] = common::rol8(table_data[i], 1) ^ key;
        key += table_data.size() - i;
    }

    io::MemoryByteStream table_stream(table_data);
    CustomArchiveEntry *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = table_stream.read_to_zero(0x0E).str();
        entry->size_orig = table_stream.read_le<u32>();
        entry->filter = table_stream.read<u8>();
        table_stream.skip(9);
        entry->offset = table_stream.read_le<u32>();
        if (last_entry)
            last_entry->size_comp = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }

    if (last_entry)
    {
        table_stream.skip(0x1C);
        last_entry->size_comp
            = table_stream.read_le<u32>() - last_entry->offset;
    }

    return meta;
}

std::unique_ptr<io::File> MrgArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size_comp);
    if (entry->filter)
    {
        if (entry->filter >= 2)
        {
            common::MrgDecryptor decryptor(data);
            data = decryptor.decrypt_without_key();
        }
        if (entry->filter < 3)
            data = common::custom_lzss_decompress(data, entry->size_orig);
    }
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> MrgArchiveDecoder::get_linked_formats() const
{
    return {"fc01/acd", "fc01/mca", "fc01/mcg"};
}

static auto _ = dec::register_decoder<MrgArchiveDecoder>("fc01/mrg");
