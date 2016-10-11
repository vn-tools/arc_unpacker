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

#include "dec/yuka_script/ykc_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::yuka_script;

static const bstr magic = "YKC001"_b;

bool YkcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> YkcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 10);
    const auto table_offset = input_file.stream.read_le<u32>();
    const auto table_size = input_file.stream.read_le<u32>();
    const auto file_count = table_size / 20;

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();

        input_file.stream.seek(table_offset + i * 20);
        const auto name_origin = input_file.stream.read_le<u32>();
        const auto name_size = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        input_file.stream.skip(4);

        input_file.stream.seek(name_origin);
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read_to_zero(name_size)).str();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> YkcArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> YkcArchiveDecoder::get_linked_formats() const
{
    return {"yuka-script/ykg"};
}

static auto _ = dec::register_decoder<YkcArchiveDecoder>("yuka-script/ykc");
