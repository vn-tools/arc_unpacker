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

#include "dec/qlie/abmp10_archive_decoder.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::qlie;

static const bstr magic10 = "abmp10\0\0\0\0\0\0\0\0\0\0"_b;
static const bstr magic11 = "abmp11\0\0\0\0\0\0\0\0\0\0"_b;
static const bstr magic12 = "abmp12\0\0\0\0\0\0\0\0\0\0"_b;
static const bstr magic_imgdat10 = "abimgdat10\0\0\0\0\0\0"_b;
static const bstr magic_imgdat11 = "abimgdat11\0\0\0\0\0\0"_b;
static const bstr magic_imgdat13 = "abimgdat13\0\0\0\0\0\0"_b;
static const bstr magic_imgdat14 = "abimgdat14\0\0\0\0\0\0"_b;
static const bstr magic_snddat10 = "absnddat10\0\0\0\0\0\0"_b;
static const bstr magic_snddat11 = "absnddat11\0\0\0\0\0\0"_b;
static const bstr magic_data10 = "abdata10\0\0\0\0\0\0\0\0"_b;
static const bstr magic_data11 = "abdata11\0\0\0\0\0\0\0\0"_b;
static const bstr magic_data12 = "abdata12\0\0\0\0\0\0\0\0"_b;
static const bstr magic_data13 = "abdata13\0\0\0\0\0\0\0\0"_b;
static const bstr magic_image10 = "abimage10\0\0\0\0\0\0\0"_b;
static const bstr magic_sound10 = "absound10\0\0\0\0\0\0\0"_b;

static int guess_version(io::BaseByteStream &input_stream)
{
    const auto magic = input_stream.read(16);
    if (magic == magic10)
        return 10;
    if (magic == magic11)
        return 11;
    if (magic == magic12)
        return 12;
    return -1;
}

static void read_data_entry(io::File &input_file, dec::ArchiveMeta &meta)
{
    auto entry = std::make_unique<dec::PlainArchiveEntry>();
    entry->path = "unknown.dat";
    entry->size = input_file.stream.read_le<u32>();
    entry->offset = input_file.stream.pos();
    input_file.stream.skip(entry->size);
    meta.entries.push_back(std::move(entry));
}

static void read_resource_entry(io::File &input_file, dec::ArchiveMeta &meta)
{
    const auto magic = input_file.stream.read(16);
    auto entry = std::make_unique<dec::PlainArchiveEntry>();
    const auto name_size = input_file.stream.read_le<u16>();
    entry->path = algo::sjis_to_utf8(input_file.stream.read(name_size)).str();
    if (entry->path.str().empty())
        entry->path = "unknown";
    entry->path.change_extension("dat");

    if (magic == magic_snddat11
        || magic == magic_imgdat11
        || magic == magic_imgdat13
        || magic == magic_imgdat14)
    {
        input_file.stream.skip(input_file.stream.read_le<u16>());
    }
    else if (magic != magic_imgdat10 && magic != magic_snddat10)
    {
        throw err::NotSupportedError(algo::format(
            "Unknown image magic: %s", magic.c_str()));
    }

    input_file.stream.skip(1);
    if (magic == magic_imgdat14)
        input_file.stream.skip(76);
    if (magic == magic_imgdat13)
        input_file.stream.skip(12);

    entry->size = input_file.stream.read_le<u32>();
    entry->offset = input_file.stream.pos();
    input_file.stream.skip(entry->size);
    if (entry->size)
        meta.entries.push_back(std::move(entry));
}

bool Abmp10ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return guess_version(input_file.stream) >= 0;
}

std::unique_ptr<dec::ArchiveMeta> Abmp10ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(16);
    auto meta = std::make_unique<ArchiveMeta>();
    while (input_file.stream.left())
    {
        const auto magic = input_file.stream.read(16);
        if (magic == magic_data10
            || magic == magic_data11
            || magic == magic_data12
            || magic == magic_data13)
        {
            read_data_entry(input_file, *meta);
        }
        else if (magic == magic_image10 || magic == magic_sound10)
        {
            const auto file_count = input_file.stream.read<u8>();
            for (const auto i : algo::range(file_count))
                read_resource_entry(input_file, *meta);
        }
        else
        {
            throw err::NotSupportedError(algo::format(
                "Unknown section: %s", magic.c_str()));
        }
    }
    return meta;
}

std::unique_ptr<io::File> Abmp10ArchiveDecoder::read_file_impl(
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

std::vector<std::string> Abmp10ArchiveDecoder::get_linked_formats() const
{
    return {"qlie/abmp7", "qlie/abmp10", "qlie/dpng"};
}

static auto _ = dec::register_decoder<Abmp10ArchiveDecoder>("qlie/abmp10");
