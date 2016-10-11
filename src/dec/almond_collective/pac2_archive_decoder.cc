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

#include "dec/almond_collective/pac2_archive_decoder.h"
#include "algo/binary.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::almond_collective;

static const bstr magic = "PAC2"_b;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        bstr key;
    };
}

bool Pac2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Pac2ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(12);
    if (input_file.stream.read(4) != "CCOD"_b)
        throw err::CorruptDataError("Expected 'CCOD' chunk");
    const auto key_size = input_file.stream.read_le<u32>();
    const auto key = input_file.stream.read(key_size);

    if (input_file.stream.read(4) != "FNUM"_b)
        throw err::CorruptDataError("Expected 'FNUM' chunk");
    const auto unk0 = input_file.stream.read_le<u32>();
    const auto file_count = input_file.stream.read_le<u32>();

    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->key = key;
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        if (input_file.stream.read(4) != "FILE"_b)
            throw err::CorruptDataError("Expected 'FILE' chunk");
        const auto entry_size = input_file.stream.read_le<u32>();
        if (input_file.stream.read(4) != "NAME"_b)
            throw err::CorruptDataError("Expected 'NAME' chunk");
        const auto name_size = input_file.stream.read_le<u32>();
        entry->path = input_file.stream.read_to_zero(name_size).str(true);
        if (input_file.stream.read(4) != "DATA"_b)
            throw err::CorruptDataError("Expected 'DATA' chunk");
        entry->size = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.pos();
        input_file.stream.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> Pac2ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size);
    auto key_pos = entry->offset;
    for (const auto i : algo::range(data.size()))
        data[i] ^= meta->key[key_pos++ % meta->key.size()];
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Pac2ArchiveDecoder::get_linked_formats() const
{
    return {"almond-collective/teyl"};
}

static auto _ = dec::register_decoder<Pac2ArchiveDecoder>(
    "almond-collective/pac2");
