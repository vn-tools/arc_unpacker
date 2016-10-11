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

#include "dec/team_shanghai_alice/pbg4_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::team_shanghai_alice;

static const bstr magic = "PBG4"_b;

static bstr decompress(const bstr &data, size_t size_orig)
{
    algo::pack::BitwiseLzssSettings settings;
    settings.position_bits = 13;
    settings.size_bits = 4;
    settings.min_match_size = 3;
    settings.initial_dictionary_pos = 1;
    return algo::pack::lzss_decompress(data, size_orig, settings);
}

bool Pbg4ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Pbg4ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    const auto table_offset = input_file.stream.read_le<u32>();
    const auto table_size_orig = input_file.stream.read_le<u32>();

    input_file.stream.seek(table_offset);
    io::MemoryByteStream table_stream(
        decompress(input_file.stream.read_to_eof(), table_size_orig));

    CompressedArchiveEntry *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CompressedArchiveEntry>();
        entry->path = table_stream.read_to_zero().str();
        entry->offset = table_stream.read_le<u32>();
        entry->size_orig = table_stream.read_le<u32>();
        table_stream.skip(4);
        if (last_entry)
            last_entry->size_comp = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }
    if (last_entry)
        last_entry->size_comp = table_offset - last_entry->offset;
    return meta;
}

std::unique_ptr<io::File> Pbg4ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CompressedArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);
    data = decompress(data, entry->size_orig);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Pbg4ArchiveDecoder::get_linked_formats() const
{
    return {"team-shanghai-alice/anm"};
}

static auto _ = dec::register_decoder<Pbg4ArchiveDecoder>(
    "team-shanghai-alice/pbg4");
