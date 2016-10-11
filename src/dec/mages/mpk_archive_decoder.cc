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

#include "dec/mages/mpk_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::mages;

static const bstr magic = "MPK\0"_b;

bool MpkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> MpkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    // only known version is 02.00
    const auto version_minor = input_file.stream.read_le<u16>();
    const auto version_major = input_file.stream.read_le<u16>();

    auto meta = std::make_unique<ArchiveMeta>();

    const auto file_count = input_file.stream.read_le<u32>();
    input_file.stream.seek(64); // unused header fields
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        // 32-bit unknown field (always 0), then 32-bit index
        input_file.stream.skip(8);
        entry->offset = input_file.stream.read_le<u64>();
        entry->size = input_file.stream.read_le<u64>();
        input_file.stream.skip(8); // duplicate of size, purpose unknown
        entry->path = input_file.stream.read_to_zero(224).str();

        // file_count appears to include some blank alignment entries
        if (entry->offset != 0 && entry->size != 0)
            meta->entries.push_back(std::move(entry));
    }

    return meta;
}

std::unique_ptr<io::File> MpkArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> MpkArchiveDecoder::get_linked_formats() const
{
    return {"microsoft/dds", "png/png"};
}

static auto _ = dec::register_decoder<MpkArchiveDecoder>("mages/mpk");
