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

#include "dec/tabito/dat_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::tabito;

bool DatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("dat"))
        return false;
    input_file.stream.seek(0);
    while (true)
    {
        const auto size = input_file.stream.read_le<u32>();
        if (size == 0)
            return input_file.stream.left() == 0;
        input_file.stream.skip(4);
        const auto name_size = input_file.stream.read_le<u16>();
        input_file.stream.skip(name_size + size);
    }
    return false;
}

std::unique_ptr<dec::ArchiveMeta> DatArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    auto meta = std::make_unique<ArchiveMeta>();
    while (true)
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->size = input_file.stream.read_le<u32>();
        if (entry->size == 0)
            break;
        input_file.stream.skip(4);
        const auto name_size = input_file.stream.read_le<u16>();
        entry->path = input_file.stream.read_to_zero(name_size).str();
        entry->offset = input_file.stream.pos();
        input_file.stream.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> DatArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    auto ret = std::make_unique<io::File>(entry->path, data);
    ret->guess_extension();
    return ret;
}

std::vector<std::string> DatArchiveDecoder::get_linked_formats() const
{
    return {"tabito/gwd"};
}

static auto _ = dec::register_decoder<DatArchiveDecoder>("tabito/dat");
