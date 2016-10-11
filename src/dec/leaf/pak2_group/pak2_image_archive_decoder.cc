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

#include "dec/leaf/pak2_group/pak2_image_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "\x5F\xF8\x6D\x75"_b;
static const bstr mask_magic = "\x03\xC5\x0D\xA6"_b;
static const bstr end_magic = "\xAF\xF6\x4D\x4E"_b;

namespace
{
    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        uoff_t color_offset, mask_offset;
        size_t size;
        size_t width, height, bpp;
    };
}

bool Pak2ImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(4).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Pak2ImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();

    input_file.stream.seek(0);
    CustomArchiveEntry *last_entry = nullptr;
    while (input_file.stream.left())
    {
        input_file.stream.skip(4);
        const auto entry_magic = input_file.stream.read(4);
        if (entry_magic == magic)
        {
            auto entry = std::make_unique<CustomArchiveEntry>();
            input_file.stream.skip(18);
            entry->bpp = input_file.stream.read_le<u16>();
            input_file.stream.skip(8);
            entry->width = input_file.stream.read_le<u16>();
            entry->height = input_file.stream.read_le<u16>();
            entry->color_offset = input_file.stream.pos();
            entry->size = entry->width * entry->height * entry->bpp >> 3;
            input_file.stream.skip(entry->size);

            if (!entry->width || !entry->height)
                continue;
            last_entry = entry.get();
            meta->entries.push_back(std::move(entry));
        }
        else if (entry_magic == mask_magic)
        {
            if (!last_entry)
                throw err::CorruptDataError("Mask found, but no color data");
            input_file.stream.skip(28);
            last_entry->mask_offset = input_file.stream.pos();
            input_file.stream.skip(last_entry->width * last_entry->height);
        }
        else if (entry_magic == end_magic)
        {
            input_file.stream.skip(12);
            if (input_file.stream.left())
                throw err::CorruptDataError("More data follow");
        }
        else
            throw err::CorruptDataError("Unexpected magic");
    }

    return meta;
}

std::unique_ptr<io::File> Pak2ImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    res::PixelFormat format;
    if (entry->bpp == 8)
        format = res::PixelFormat::Gray8;
    else if (entry->bpp == 16)
        format = res::PixelFormat::BGRnA5551;
    else
        throw err::UnsupportedBitDepthError(entry->bpp);

    const auto data = input_file.stream
        .seek(entry->color_offset)
        .read(entry->size);
    auto image = res::Image(entry->width, entry->height, data, format);
    image.flip_vertically();
    if (entry->mask_offset)
    {
        const auto mask_data = input_file
            .stream.seek(entry->mask_offset)
            .read(entry->width * entry->height);
        const auto mask = res::Image(
            entry->width, entry->height, mask_data, res::PixelFormat::Gray8);
        image.apply_mask(mask);
    }
    const auto encoder = enc::png::PngImageEncoder();
    return encoder.encode(logger, image, entry->path);
}

algo::NamingStrategy Pak2ImageArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Sibling;
}

static auto _ = dec::register_decoder<Pak2ImageArchiveDecoder>(
    "leaf/pak2-image");
