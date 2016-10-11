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

#include "dec/alice_soft/dat_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "algo/str.h"
#include "err.h"

using namespace au;
using namespace au::dec::alice_soft;

bool DatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    Logger dummy_logger;
    dummy_logger.mute();
    try
    {
        read_meta(dummy_logger, input_file);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::unique_ptr<dec::ArchiveMeta> DatArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto arc_name = algo::lower(input_file.path.stem());
    const auto header_size = (input_file.stream.read_le<u16>() - 1) * 256;
    auto meta = std::make_unique<ArchiveMeta>();

    PlainArchiveEntry *last_entry = nullptr;
    bool finished = false;
    for (const auto i : algo::range((header_size / 2) - 1))
    {
        auto offset = input_file.stream.read_le<u16>();
        if (offset == 0)
        {
            finished = true;
            continue;
        }
        offset = (offset - 1) * 256;

        if (last_entry && offset < last_entry->offset)
            throw err::CorruptDataError("Expected offsets to be sorted");
        if (finished)
            throw err::CorruptDataError("Expected remaining offsets to be 0");
        if (offset > input_file.stream.size())
            throw err::BadDataOffsetError();
        if (offset == input_file.stream.size())
            continue;

        // necessary for VSP recognition
        std::string ext = "dat";
        if (arc_name.find("cg") != std::string::npos)
            ext = "vsp";
        else if (arc_name.find("dis") != std::string::npos)
            ext = "sco";
        else if (arc_name.find("mus") != std::string::npos)
            ext = "mus";
        else if (arc_name.find("map") != std::string::npos)
            ext = "map";

        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->offset = offset;
        entry->path = algo::format("%03d.%s", i, ext.c_str());
        if (last_entry)
            last_entry->size = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }

    if (last_entry)
        last_entry->size = input_file.stream.size() - last_entry->offset;
    else
        throw err::CorruptDataError("File table cannot be empty");

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
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> DatArchiveDecoder::get_linked_formats() const
{
    return {"alice-soft/vsp"};
}

static auto _ = dec::register_decoder<DatArchiveDecoder>("alice-soft/dat");
