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

#include "dec/will/pnap_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::will;

static const bstr magic = "PNAP"_b;

bool PnapArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> PnapArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(16);
    const auto file_count = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        input_file.stream.skip(36);
        entry->size = input_file.stream.read_le<u32>();
        if (entry->size)
            meta->entries.push_back(std::move(entry));
    }
    auto current_offset = input_file.stream.pos();
    for (const auto &e : meta->entries)
    {
        auto entry = static_cast<PlainArchiveEntry*>(e.get());
        entry->offset = current_offset;
        current_offset += entry->size;
    }
    return meta;
}

std::unique_ptr<io::File> PnapArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

static auto _ = dec::register_decoder<PnapArchiveDecoder>("will/pnap");
