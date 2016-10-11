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

#include "dec/kaguya/base_link_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "dec/kaguya/common/params_encryption.h"
#include "err.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::kaguya;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        common::Params params;
    };

    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        bool encrypted;
    };
}

std::unique_ptr<dec::ArchiveMeta> BaseLinkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<CustomArchiveMeta>();
    auto params_file = VirtualFileSystem::get_by_name("params.dat");
    if (params_file)
    {
        try
        {
            meta->params = common::parse_params_file(params_file->stream);
        }
        catch (const std::exception &e)
        {
            logger.warn("%s\n", e.what());
        }
    }

    if (get_version() == 3)
    {
        input_file.stream.seek(8);
    }
    else if (get_version() == 4 || get_version() == 5)
    {
        input_file.stream.seek(10);
    }
    else if (get_version() == 6)
    {
        input_file.stream.seek(7);
        const auto name_size = input_file.stream.read<u8>();
        const auto name = input_file.stream.read(name_size);
    }
    else
    {
        throw std::logic_error("Bad version");
    }

    while (true)
    {
        const auto entry_offset = input_file.stream.pos();
        const auto entry_size = input_file.stream.read_le<u32>();
        if (!entry_size)
            break;

        auto entry = std::make_unique<CustomArchiveEntry>();
        const auto flags = input_file.stream.read_le<u16>();
        entry->encrypted = flags & 4;
        input_file.stream.skip(7);

        if (get_version() == 3 || get_version() == 4 || get_version() == 5)
        {
            const auto name_size = input_file.stream.read<u8>();
            input_file.stream.skip(2);
            entry->path = algo::sjis_to_utf8(
                input_file.stream.read(name_size)).str();
        }
        else if (get_version() == 6)
        {
            const auto name_size = input_file.stream.read_le<u16>();
            entry->path = algo::utf16_to_utf8(
                input_file.stream.read(name_size)).str();
        }

        entry->offset = input_file.stream.pos();
        entry->size = entry_size - (entry->offset - entry_offset);
        meta->entries.push_back(std::move(entry));

        input_file.stream.seek(entry_offset + entry_size);
    }

    return std::move(meta);
}

std::unique_ptr<io::File> BaseLinkArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    auto output_file = std::make_unique<io::File>(entry->path, ""_b);
    output_file->stream.write(
        input_file.stream.seek(entry->offset), entry->size);

    if (entry->encrypted)
    {
        if (meta->params.key.empty())
            throw err::CorruptDataError("Missing decryption params");
        common::decrypt(output_file->stream, meta->params);
    }
    return output_file;
}

std::vector<std::string> BaseLinkArchiveDecoder::get_linked_formats() const
{
    return
    {
        "kaguya/ap",
        "kaguya/ap0",
        "kaguya/ap2",
        "kaguya/ap3",
        "kaguya/an00",
        "kaguya/an10",
        "kaguya/an20",
        "kaguya/an21",
        "kaguya/bmr",
        "kaguya/pl00",
        "kaguya/pl10",
        "microsoft/bmp"
    };
}
