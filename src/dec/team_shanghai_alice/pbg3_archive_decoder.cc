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

#include "dec/team_shanghai_alice/pbg3_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::team_shanghai_alice;

static const bstr magic = "PBG3"_b;

namespace
{
    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        u32 checksum;
    };
}

static unsigned int read_integer(io::BaseBitStream &bit_stream)
{
    const auto integer_size = bit_stream.read(2) + 1;
    return bit_stream.read(integer_size << 3);
}

bool Pbg3ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Pbg3ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());

    io::MsbBitStream header_bit_stream(input_file.stream);
    const auto file_count = read_integer(header_bit_stream);
    const auto table_offset = read_integer(header_bit_stream);

    input_file.stream.seek(table_offset);
    io::MsbBitStream table_bit_stream(input_file.stream);

    CustomArchiveEntry *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        read_integer(table_bit_stream);
        read_integer(table_bit_stream);
        entry->checksum = read_integer(table_bit_stream);
        entry->offset = read_integer(table_bit_stream);
        entry->size_orig = read_integer(table_bit_stream);
        std::string name;
        for (const auto j : algo::range(256))
        {
            char c = table_bit_stream.read(8);
            if (c == 0)
                break;
            name += c;
        }
        entry->path = name;
        if (last_entry)
            last_entry->size_comp = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }
    if (last_entry)
        last_entry->size_comp = table_offset - last_entry->offset;
    return meta;
}

std::unique_ptr<io::File> Pbg3ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    io::MsbBitStream bit_stream(input_file.stream.read(entry->size_comp));

    algo::pack::BitwiseLzssSettings settings;
    settings.position_bits = 13;
    settings.size_bits = 4;
    settings.min_match_size = 3;
    settings.initial_dictionary_pos = 1;
    const auto data = algo::pack::lzss_decompress(
        bit_stream, entry->size_orig, settings);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Pbg3ArchiveDecoder::get_linked_formats() const
{
    return {"team-shanghai-alice/anm"};
}

static auto _ = dec::register_decoder<Pbg3ArchiveDecoder>(
    "team-shanghai-alice/pbg3");
