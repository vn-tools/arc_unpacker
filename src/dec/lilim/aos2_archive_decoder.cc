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

#include "dec/lilim/aos2_archive_decoder.h"
#include "algo/range.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::lilim;

bool Aos2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    if (input_file.stream.read_le<u32>() != 0)
        return false;
    const auto data_offset = input_file.stream.read_le<u32>();
    input_file.stream.seek(data_offset - 8);
    const auto last_entry_offset = input_file.stream.read_le<u32>();
    const auto last_entry_size = input_file.stream.read_le<u32>();
    const auto expected_size
        = data_offset + last_entry_offset + last_entry_size;
    return input_file.stream.size() == expected_size;
}

std::unique_ptr<dec::ArchiveMeta> Aos2ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(4);
    const auto data_offset = input_file.stream.read_le<u32>();
    const auto table_size = input_file.stream.read_le<u32>();
    const auto file_count = table_size / 0x28;
    auto meta = std::make_unique<ArchiveMeta>();
    input_file.stream.seek(data_offset - table_size);
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = input_file.stream.read_to_zero(0x20).str();
        entry->offset = input_file.stream.read_le<u32>() + data_offset;
        entry->size = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Aos2ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Aos2ArchiveDecoder::get_linked_formats() const
{
    return {"lilim/scr", "lilim/abm", "microsoft/bmp"};
}

static auto _ = dec::register_decoder<Aos2ArchiveDecoder>("lilim/aos2");
