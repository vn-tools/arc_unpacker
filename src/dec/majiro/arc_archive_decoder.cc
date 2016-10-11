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

#include "dec/majiro/arc_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "algo/str.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::majiro;

static const bstr magic = "MajiroArcV"_b;

bool ArcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> ArcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto version = algo::from_string<float>(
        input_file.stream.read_to_zero().str());
    const auto file_count = input_file.stream.read_le<u32>();
    const auto names_offset = input_file.stream.read_le<u32>();
    const auto data_offset = input_file.stream.read_le<u32>();

    if (version != 2 && version != 3)
        throw err::UnsupportedVersionError(version);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        const auto hash = version == 3
            ? input_file.stream.read_le<u64>()
            : input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }

    input_file.stream.seek(names_offset);
    for (auto &entry : meta->entries)
    {
        static_cast<PlainArchiveEntry*>(entry.get())->path
            = algo::sjis_to_utf8(input_file.stream.read_to_zero()).str();
    }

    return meta;
}

std::unique_ptr<io::File> ArcArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> ArcArchiveDecoder::get_linked_formats() const
{
    return {"majiro/rc8", "majiro/rct"};
}

static auto _ = dec::register_decoder<ArcArchiveDecoder>("majiro/arc");
