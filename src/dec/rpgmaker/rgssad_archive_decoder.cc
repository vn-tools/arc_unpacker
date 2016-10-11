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

#include "dec/rpgmaker/rgssad_archive_decoder.h"
#include "algo/range.h"
#include "dec/rpgmaker/rgs/common.h"

using namespace au;
using namespace au::dec::rpgmaker;

static const bstr magic = "RGSSAD\x00\x01"_b;
static const u32 initial_key = 0xDEADCAFE;

bool RgssadArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> RgssadArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    auto key = initial_key;
    auto meta = std::make_unique<ArchiveMeta>();
    while (input_file.stream.left())
    {
        auto entry = std::make_unique<rgs::CustomArchiveEntry>();

        const auto name_size = input_file.stream.read_le<u32>() ^ key;
        key = rgs::advance_key(key);
        auto name = input_file.stream.read(name_size).str();
        for (const auto i : algo::range(name_size))
        {
            name[i] ^= key;
            key = rgs::advance_key(key);
        }
        entry->path = name;

        entry->size = input_file.stream.read_le<u32>() ^ key;
        key = rgs::advance_key(key);

        entry->key = key;
        entry->offset = input_file.stream.pos();

        input_file.stream.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> RgssadArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    return rgs::read_file_impl(
        input_file, *static_cast<const rgs::CustomArchiveEntry*>(&e));
}

static auto _ = dec::register_decoder<RgssadArchiveDecoder>("rpgmaker/rgssad");
