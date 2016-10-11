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

#include "dec/aoi/vfs_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::aoi;

static const bstr magic = "VF"_b;

bool VfsArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.seek(0).read(magic.size()) != magic)
        return false;
    const auto version = input_file.stream.read_le<u16>();
    return version == 0x100 || version == 0x101 || version == 0x200;
}

std::unique_ptr<dec::ArchiveMeta> VfsArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(2);
    const auto version = input_file.stream.read_le<u16>();
    auto meta = std::make_unique<ArchiveMeta>();
    const auto file_count = input_file.stream.read_le<u16>();
    const auto entry_size = input_file.stream.read_le<u16>();
    const auto table_size = input_file.stream.read_le<u32>();
    const auto file_size = input_file.stream.read_le<u32>();

    if (version == 0x100 || version == 0x101)
    {
        for (const auto i : algo::range(file_count))
        {
            const auto entry_offset = input_file.stream.pos();
            auto entry = std::make_unique<PlainArchiveEntry>();
            entry->path = input_file.stream.read_to_zero(0x13).str();
            entry->offset = input_file.stream.read_le<u32>();
            entry->size = input_file.stream.read_le<u32>();
            meta->entries.push_back(std::move(entry));
            input_file.stream.seek(entry_offset + entry_size);
        }
    }

    else if (version == 0x200)
    {
        const auto names_offset
            = input_file.stream.pos() + entry_size * file_count + 8;
        for (const auto i : algo::range(file_count))
        {
            const auto entry_offset = input_file.stream.pos();
            auto entry = std::make_unique<PlainArchiveEntry>();

            const auto name_offset
                = names_offset + input_file.stream.read_le<u32>() * 2;
            input_file.stream.skip(6);
            entry->offset = input_file.stream.read_le<u32>();
            entry->size = input_file.stream.read_le<u32>();

            input_file.stream.seek(name_offset);
            bstr name_utf16;
            while (true)
            {
                const auto chunk = input_file.stream.read(2);
                if (chunk == "\x00\x00"_b)
                    break;
                name_utf16 += chunk;
            }
            entry->path = algo::utf16_to_utf8(name_utf16).str();

            meta->entries.push_back(std::move(entry));
            input_file.stream.seek(entry_offset + entry_size);
        }
    }
    else
    {
        throw err::UnsupportedVersionError(version);
    }

    return meta;
}

std::unique_ptr<io::File> VfsArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> VfsArchiveDecoder::get_linked_formats() const
{
    return {"aoi/iph", "aoi/aog", "aoi/agf", "microsoft/dds"};
}

static auto _ = dec::register_decoder<VfsArchiveDecoder>("aoi/vfs");
