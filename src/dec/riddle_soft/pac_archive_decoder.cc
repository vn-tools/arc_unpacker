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

#include "dec/riddle_soft/pac_archive_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::riddle_soft;

static const bstr magic = "PAC1"_b;

bool PacArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> PacArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    const auto file_data_start = input_file.stream.pos() + file_count * 32;
    auto current_file_offset = 0;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = input_file.stream.read_to_zero(16).str();
        entry->size = input_file.stream.read_le<u32>();
        const auto prefix = input_file.stream.read(4);
        const auto unk1 = input_file.stream.read_le<u32>();
        const auto unk2 = input_file.stream.read_le<u32>();
        if (unk1 != unk2)
            throw err::CorruptDataError("Data mismatch");
        entry->offset = file_data_start + current_file_offset;
        current_file_offset += entry->size;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> PacArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> PacArchiveDecoder::get_linked_formats() const
{
    return {"riddle-soft/cmp"};
}

static auto _ = dec::register_decoder<PacArchiveDecoder>("riddle-soft/pac");
