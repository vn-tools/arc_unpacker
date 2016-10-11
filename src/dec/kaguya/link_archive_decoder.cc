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

#include "dec/kaguya/link_archive_decoder.h"
#include "algo/range.h"
#include "dec/kaguya/common/custom_lzss.h"

using namespace au;
using namespace au::dec::kaguya;

static const auto magic = "LINK"_b;

bool LinkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.seek(0).read(magic.size()) != magic)
        return false;
    const auto file_count = input_file.stream.read_le<u32>();
    const auto file_names_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(file_names_size);
    input_file.stream.skip(8 * (file_count - 1));
    const auto last_offset = input_file.stream.read_le<u32>();
    const auto last_size = input_file.stream.read_le<u32>();
    return last_offset + last_size == input_file.stream.size();
}

std::unique_ptr<dec::ArchiveMeta> LinkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    const auto file_names_size = input_file.stream.read_le<u32>();
    const auto file_names_start = input_file.stream.pos();

    std::vector<bstr> file_names;
    for (const auto i : algo::range(file_count))
        file_names.push_back(input_file.stream.read_to_zero());

    input_file.stream.seek(file_names_start + file_names_size);
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = file_names.at(i).str();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> LinkArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);

    bstr data;
    if (entry->path.has_extension("cg_")
        || entry->path.has_extension("bg_")
        || entry->path.has_extension("sp_"))
    {
        const auto size_orig = input_file.stream.read_le<u32>();
        data = input_file.stream.read(entry->size - 4);
        data = common::custom_lzss_decompress(data, size_orig);
    }
    else
    {
        data = input_file.stream.read(entry->size);
    }

    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> LinkArchiveDecoder::get_linked_formats() const
{
    return {"microsoft/bmp"};
}

static auto _ = dec::register_decoder<LinkArchiveDecoder>("kaguya/link");
