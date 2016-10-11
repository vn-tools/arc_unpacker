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

#include "dec/real_live/nwk_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::real_live;

bool NwkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("nwk");
}

std::unique_ptr<dec::ArchiveMeta> NwkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto file_count = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->size = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.read_le<u32>();
        const auto file_id = input_file.stream.read_le<u32>();
        entry->path = algo::format("sample%05d", file_id);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> NwkArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(
        io::path(entry->path).change_extension("nwa"), data);
}

std::vector<std::string> NwkArchiveDecoder::get_linked_formats() const
{
    return {"real-live/nwa"};
}

static auto _ = dec::register_decoder<NwkArchiveDecoder>("real-live/nwk");
