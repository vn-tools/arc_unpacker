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

#include "dec/wild_bug/wbp_archive_decoder.h"
#include <map>
#include "algo/range.h"

using namespace au;
using namespace au::dec::wild_bug;

static const bstr magic = "ARCFORM4\x20WBUG\x20"_b;

bool WbpArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> WbpArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0x10);
    const auto file_count = input_file.stream.read_le<u32>();
    const auto table_offset = input_file.stream.read_le<u32>();
    const auto table_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(8);

    std::vector<uoff_t> dir_offsets;
    for (const auto i : algo::range(0x100))
    {
        const auto offset = input_file.stream.read_le<u32>();
        if (offset)
            dir_offsets.push_back(offset);
    }

    std::vector<uoff_t> file_offsets;
    for (const auto i : algo::range(0x100))
    {
        const auto offset = input_file.stream.read_le<u32>();
        if (offset)
            file_offsets.push_back(offset);
    }

    std::map<u16, std::string> dir_names;
    for (const auto &offset : dir_offsets)
    {
        input_file.stream.seek(offset + 1);
        const auto name_size = input_file.stream.read<u8>();
        const auto dir_id = input_file.stream.read<u8>();
        input_file.stream.skip(1);
        dir_names[dir_id] = input_file.stream.read_to_zero(name_size).str();
    }

    auto meta = std::make_unique<ArchiveMeta>();
    for (const size_t i : algo::range(file_offsets.size()))
    {
        // one file offset may contain multiple entries
        input_file.stream.seek(file_offsets[i]);
        do
        {
            const auto old_pos = input_file.stream.pos();

            input_file.stream.skip(1);
            const auto name_size = input_file.stream.read<u8>();
            const auto dir_id = input_file.stream.read<u8>();
            input_file.stream.skip(1);

            auto entry = std::make_unique<PlainArchiveEntry>();
            entry->offset = input_file.stream.read_le<u32>();
            entry->size = input_file.stream.read_le<u32>();
            input_file.stream.skip(8);
            entry->path = dir_names.at(dir_id)
                + input_file.stream.read_to_zero(name_size).str();

            meta->entries.push_back(std::move(entry));

            input_file.stream.seek(old_pos + (name_size & 0xFC) + 0x18);
        }
        while (i + 1 < file_offsets.size()
            && input_file.stream.pos() < file_offsets[i + 1]);
    }
    return meta;
}

std::unique_ptr<io::File> WbpArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> WbpArchiveDecoder::get_linked_formats() const
{
    return {"wild-bug/wbi", "wild-bug/wbm", "wild-bug/wpn", "wild-bug/wwa"};
}

static auto _ = dec::register_decoder<WbpArchiveDecoder>("wild-bug/wbp");
