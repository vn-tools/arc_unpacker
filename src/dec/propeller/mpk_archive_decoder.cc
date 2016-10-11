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

#include "dec/propeller/mpk_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::propeller;

bool MpkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("mpk");
}

std::unique_ptr<dec::ArchiveMeta> MpkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto table_offset = input_file.stream.read_le<u32>();
    const auto file_count = input_file.stream.read_le<u32>();

    input_file.stream.seek(table_offset);
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();

        auto name_bin = input_file.stream.read(32);
        const u8 key8 = name_bin[31];
        const u32 key32 = (key8 << 24) | (key8 << 16) | (key8 << 8) | key8;

        for (auto &c : name_bin)
            c ^= key8;

        auto name = algo::sjis_to_utf8(name_bin).str(true);
        if (name[0] == '\\')
            name = name.substr(1);
        entry->path = name;

        entry->offset = input_file.stream.read_le<u32>() ^ key32;
        entry->size = input_file.stream.read_le<u32>() ^ key32;

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
    return {"propeller/mgr"};
}

static auto _ = dec::register_decoder<MpkArchiveDecoder>("propeller/mpk");
