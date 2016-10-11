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

#include "dec/playstation/gpda_archive_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::playstation;

static const bstr magic = "GPDA"_b;

bool GpdaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    return input_file.stream.read(magic.size()) == magic
        && input_file.stream.read_le<u32>() == input_file.stream.size();
}

std::unique_ptr<dec::ArchiveMeta> GpdaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(12);
    const auto file_count = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();

        entry->offset = input_file.stream.read_le<u32>();
        if (input_file.stream.read_le<u32>() != 0)
            throw err::CorruptDataError("Expected '0'");
        entry->size = input_file.stream.read_le<u32>();

        const auto name_offset = input_file.stream.read_le<u32>();
        if (!name_offset)
            entry->path = "";
        else
            input_file.stream.peek(name_offset, [&]()
                {
                    const auto name_size = input_file.stream.read_le<u32>();
                    entry->path = input_file.stream.read(name_size).str();
                });

        if (!entry->size)
            continue;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> GpdaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> GpdaArchiveDecoder::get_linked_formats() const
{
    return {"playstation/gpda", "playstation/gim", "cri/hca", "gnu/gzip"};
}

static auto _ = dec::register_decoder<GpdaArchiveDecoder>("playstation/gpda");
