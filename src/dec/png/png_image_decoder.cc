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

#include "dec/png/png_image_decoder.h"
#include <cstring>
#include <png.h>
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::png;

static const bstr magic = "\x89PNG"_b;

static void read_handler(png_structp png_ptr, png_bytep output, png_size_t size)
{
    auto input_stream
        = reinterpret_cast<io::BaseByteStream*>(png_get_io_ptr(png_ptr));
    const auto input = input_stream->read(size);
    std::memcpy(output, input.get<u8>(), size);
}

static int custom_chunk_handler(png_structp png_ptr, png_unknown_chunkp chunk)
{
    const auto handler = reinterpret_cast<PngImageDecoder::ChunkHandler*>(
        png_get_user_chunk_ptr(png_ptr));
    const auto name = std::string(reinterpret_cast<const char*>(chunk->name));
    const auto data = bstr(chunk->data, chunk->size);
    (*handler)(name, data);
    return 1; // == handled
}

static res::Image decode(
    const Logger &logger, io::File &file, PngImageDecoder::ChunkHandler handler)
{
    png_structp png_ptr = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        throw std::logic_error("Failed to create PNG read structure");

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        throw std::logic_error("Failed to create PNG info structure");

    png_set_error_fn(
        png_ptr,
        png_get_error_ptr(png_ptr),
        [](png_structp png_ptr, png_const_charp error_msg)
        {
            throw err::CorruptDataError(error_msg);
        },
        [](png_structp png_ptr, png_const_charp warning_msg)
        {
            // TODO
            Logger logger;
            logger.warn("libpng warning: %s\n", warning_msg);
        });

    png_set_read_user_chunk_fn(png_ptr, &handler, custom_chunk_handler);
    png_set_read_fn(png_ptr, &file.stream, &read_handler);
    png_read_png(
        png_ptr,
        info_ptr,
        PNG_TRANSFORM_GRAY_TO_RGB
            | PNG_TRANSFORM_STRIP_16
            | PNG_TRANSFORM_PACKING
            | PNG_TRANSFORM_BGR
            | PNG_TRANSFORM_EXPAND,
        nullptr);

    int color_type;
    int bits_per_channel;
    png_uint_32 width, height;
    png_get_IHDR(
        png_ptr, info_ptr,
        &width, &height,
        &bits_per_channel, &color_type,
        nullptr, nullptr, nullptr);
    if (bits_per_channel != 8)
        throw err::UnsupportedBitDepthError(bits_per_channel);

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    res::PixelFormat format;
    if (color_type == PNG_COLOR_TYPE_RGB)
        format = res::PixelFormat::BGR888;
    else if (color_type == PNG_COLOR_TYPE_RGBA)
        format = res::PixelFormat::BGRA8888;
    else if (color_type == PNG_COLOR_TYPE_GRAY)
        format = res::PixelFormat::Gray8;
    else
        throw err::NotSupportedError("Bad pixel format");

    bstr data;
    data.reserve(width * height * pixel_format_to_bpp(format));
    for (const auto y : algo::range(height))
        data += bstr(row_pointers[y], width * pixel_format_to_bpp(format));
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

    return res::Image(width, height, data, format);
}

bool PngImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image PngImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    return ::decode(
        logger, input_file, [&](const std::string &name, const bstr &data)
        {
            logger.warn("Ignoring unknown PNG chunk: %s\n", name.c_str());
        });
}

res::Image PngImageDecoder::decode(
    const Logger &logger,
    io::File &input_file,
    const PngImageDecoder::ChunkHandler chunk_handler) const
{
    return ::decode(logger, input_file, chunk_handler);
}

static auto _ = dec::register_decoder<PngImageDecoder>("png/png");
