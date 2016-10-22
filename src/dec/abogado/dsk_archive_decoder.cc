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

#include "dec/abogado/dsk_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "err.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::abogado;

bool DskArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("DSK");
}

std::unique_ptr<dec::ArchiveMeta> DskArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto base_file = VirtualFileSystem::get_by_name(
        io::path(input_file.path).change_extension("PFT").name());
    if (!base_file)
        throw err::RecognitionError("Missing file table file");
    base_file->stream.seek(0);

    const auto version = base_file->stream.read_le<u32>();
    if (version != 0x08000010)
        throw err::UnsupportedVersionError(version);

    const auto file_count = base_file->stream.read_le<u32>();
    base_file->stream.skip(8);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = algo::sjis_to_utf8(
            base_file->stream.read_to_zero(8)).str();
        entry->offset = base_file->stream.read_le<u32>() << 11;
        entry->size = base_file->stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> DskArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> DskArchiveDecoder::get_linked_formats() const
{
    return {"abogado/kg", "abogado/v"};
}

static auto _ = dec::register_decoder<DskArchiveDecoder>("abogado/dsk");
