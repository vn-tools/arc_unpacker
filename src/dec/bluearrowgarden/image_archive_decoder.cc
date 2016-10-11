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

#include "dec/bluearrowgarden/image_archive_decoder.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"

using namespace au;
using namespace au::dec::bluearrowgarden;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        std::unique_ptr<res::Image> image;
    };

    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        float width, height;
        struct
        {
            float x, y;
        } rect[4];
    };
}

bool ImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(4);
    const auto file_count = input_file.stream.read_le<u32>();
    const auto bitmap_offset = input_file.stream.read_le<u16>();
    return bitmap_offset == 16 + file_count * 40;
}

std::unique_ptr<dec::ArchiveMeta> ImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto bitmap_size = input_file.stream.read_le<u32>();
    const auto file_count = input_file.stream.read_le<u32>();
    const auto bitmap_offset = input_file.stream.read_le<u16>();
    const auto bitmap_width = input_file.stream.read_le<u16>();
    const auto bitmap_height = input_file.stream.read_le<u16>();
    const auto bitmap_type = input_file.stream.read_le<u16>();

    const auto table_pos = input_file.stream.pos();

    input_file.stream.seek(bitmap_offset);
    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->image = std::make_unique<res::Image>(
        bitmap_width,
        bitmap_height,
        input_file.stream.read(bitmap_size),
        res::PixelFormat::BGRA8888);

    std::vector<std::string> file_names;
    input_file.stream.seek(bitmap_offset + bitmap_size);
    for (const auto i : algo::range(file_count))
        file_names.push_back(input_file.stream.read_to_zero().str());

    input_file.stream.seek(table_pos);
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = file_names.at(i);
        entry->width = input_file.stream.read_le<f32>();
        entry->height = input_file.stream.read_le<f32>();
        for (const auto j : algo::range(4))
        {
            entry->rect[j].x = input_file.stream.read_le<f32>();
            entry->rect[j].y = input_file.stream.read_le<f32>();
        }
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> ImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    res::Image image(*meta->image);
    image.offset(
        -entry->rect[0].x * meta->image->width(),
        -entry->rect[0].y * meta->image->height());
    image.crop(entry->width, entry->height);
    return enc::png::PngImageEncoder().encode(logger, image, entry->path);
}

static auto _
    = dec::register_decoder<ImageArchiveDecoder>("bluearrowgarden/images");
