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

#include "dec/libido/egr_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::libido;

namespace
{
    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        uoff_t offset;
        size_t width, height;
    };
}

bool EgrArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("egr");
}

std::unique_ptr<dec::ArchiveMeta> EgrArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto i = 0;
    auto meta = std::make_unique<ArchiveMeta>();
    while (input_file.stream.left())
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->width = input_file.stream.read_le<u32>();
        entry->height = input_file.stream.read_le<u32>();
        if (input_file.stream.read_le<u32>() != entry->width * entry->height)
            throw err::BadDataSizeError();
        entry->offset = input_file.stream.pos();
        input_file.stream.skip(0x574 + entry->width * entry->height);
        entry->path = algo::format("Image%03d.png", i++);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> EgrArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    res::Palette palette(256);
    for (const auto i : algo::range(palette.size()))
    {
        input_file.stream.skip(1);
        palette[i].a = 0xFF;
        palette[i].b = input_file.stream.read<u8>();
        palette[i].r = input_file.stream.read<u8>();
        palette[i].g = input_file.stream.read<u8>();
    }

    input_file.stream.skip(0x174);
    res::Image image(
        entry->width,
        entry->height,
        input_file.stream.read(entry->width * entry->height),
        palette);

    const auto encoder = enc::png::PngImageEncoder();
    return encoder.encode(logger, image, entry->path);
}

static auto _ = dec::register_decoder<EgrArchiveDecoder>("libido/egr");
