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

#include "dec/bishop/bsa_archive_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::bishop;

static const bstr magic = "BSArc\x00\x00\x00"_b;

bool BsaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

static bool process_directory(
    io::path &current_directory, const std::string &name)
{
    if (name.size() > 0 && name[0] == '>')
    {
        current_directory /= name.substr(1);
        return true;
    }
    else if (name == "<")
    {
        current_directory = current_directory.parent();
        return true;
    }
    return false;
}

std::unique_ptr<dec::ArchiveMeta> BsaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto version = input_file.stream.read_le<u16>();
    const auto file_count = input_file.stream.read_le<u16>();
    const auto table_offset = input_file.stream.read_le<u32>();
    const auto names_offset = table_offset + file_count * 12;
    input_file.stream.seek(table_offset);

    io::path current_directory = "";
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();

        if (version == 2)
        {
            auto name = input_file.stream.read_to_zero(32).str();
            if (!process_directory(current_directory, name))
            {
                entry->offset = input_file.stream.read_le<u32>();
                entry->size = input_file.stream.read_le<u32>();
                entry->path = current_directory / name;
                meta->entries.push_back(std::move(entry));
            }
        }
        else if (version == 3)
        {
            auto file_name_offset = input_file.stream.read_le<u32>();
            entry->offset = input_file.stream.read_le<u32>();
            entry->size = input_file.stream.read_le<u32>();

            std::string name = "";
            input_file.stream.peek(names_offset + file_name_offset, [&]()
            {
                name = input_file.stream.read_to_zero().str();
            });

            if (!process_directory(current_directory, name))
            {
                entry->path = current_directory / name;
                meta->entries.push_back(std::move(entry));
            }
        }
        else
        {
            throw err::UnsupportedVersionError(version);
        }
    }
    return meta;
}

std::unique_ptr<io::File> BsaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> BsaArchiveDecoder::get_linked_formats() const
{
    return {"bishop/bsc", "bishop/bsg"};
}

static auto _ = dec::register_decoder<BsaArchiveDecoder>("bishop/bsa");
