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

#include "enc/png/png_image_encoder.h"
#include <png.h>
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::enc::png;

static void write_handler(
    png_structp png_ptr, png_bytep input, png_size_t size)
{
    auto output_stream
        = reinterpret_cast<io::BaseByteStream*>(png_get_io_ptr(png_ptr));
    output_stream->write(bstr(input, size));
}

static void flush_handler(png_structp)
{
}

void PngImageEncoder::encode_impl(
    const Logger &logger,
    const res::Image &input_image,
    io::File &output_file) const
{
    png_structp png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        throw std::logic_error("Failed to create PNG write structure");

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        throw std::logic_error("Failed to create PNG info structure");

    const auto width = input_image.width();
    const auto height = input_image.height();
    if (!width || !height)
        throw err::BadDataSizeError();
    const auto bpp = 4;
    const auto color_type = PNG_COLOR_TYPE_RGBA;
    int transformations = PNG_TRANSFORM_BGR;

    png_set_IHDR(
        png_ptr, info_ptr, width, height, 8, color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE);

    // 0 = no compression, 9 = max compression
    // 1 produces good file size and is still fast.
    png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
    png_set_compression_level(png_ptr, 1);

    png_set_write_fn(
        png_ptr, &output_file.stream, &write_handler, &flush_handler);
    png_write_info(png_ptr, info_ptr);

    auto rows = std::make_unique<const u8*[]>(height);
    for (const auto y : algo::range(height))
        rows.get()[y] = reinterpret_cast<const u8*>(&input_image.at(0, y));
    png_set_rows(png_ptr, info_ptr, const_cast<u8**>(rows.get()));
    png_write_png(png_ptr, info_ptr, transformations, nullptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    output_file.path.change_extension("png");
}
