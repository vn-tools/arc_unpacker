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

#include "dec/mebius/mcg_image_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::mebius;

static const bstr magic = "MCG"_b;

namespace
{
    struct FormatInfo final
    {
        res::PixelFormat fmt;
        bool compressed;
        bool deinterleaved;
    };
}

static FormatInfo get_format_info(const u8 type)
{
    if (type == 0)      return {res::PixelFormat::BGR888,   false,  true};
    else if (type == 1) return {res::PixelFormat::BGRA8888, false,  false};
    else if (type == 2) return {res::PixelFormat::BGR888,   true,   true};
    else if (type == 3) return {res::PixelFormat::BGRA8888, true,   true};
    else if (type == 4) return {res::PixelFormat::Gray8,    false,  false};
    else if (type == 5) return {res::PixelFormat::Gray8,    true,   false};
    else throw std::logic_error("Invalid type");
}

static bstr rle_decompress(
    io::BaseByteStream &input_stream,
    const size_t width,
    const size_t height,
    const size_t channels)
{
    bstr output(width * height * channels);
    auto output_ptr = algo::make_ptr(output);
    auto marker = input_stream.read<u8>();
    auto current_channel = 0u;

    while (input_stream.left() && output_ptr.left())
    {
        const auto control = input_stream.read<u8>();
        if (control == marker)
        {
            auto repetitions = input_stream.read<u8>();
            const auto byte = input_stream.read<u8>();
            while (repetitions-- && output_ptr.left())
                *output_ptr++ = byte;
        }
        else
        {
            *output_ptr++ = control;
        }
    }

    return output;
}

static bstr interleave(
    const bstr &input,
    const size_t width,
    const size_t height,
    const size_t channels)
{
    bstr output(width * height * channels);
    for (const auto c : algo::range(channels))
    for (const auto y : algo::range(height))
    for (const auto x : algo::range(width))
        output[(y * width + x) * channels + c]
            = input[y * width + x + c * width * height];
    return output;
}

bool McgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    const auto type = input_file.stream.read<u8>();
    return type < 8;
}

res::Image McgImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto type = input_file.stream.read<u8>();
    const auto total_width = input_file.stream.read_be<u16>();
    const auto total_height = input_file.stream.read_be<u16>();
    const auto x = input_file.stream.read_be<u16>();
    const auto y = input_file.stream.read_be<u16>();
    const auto width = input_file.stream.read_be<u16>();
    const auto height = input_file.stream.read_be<u16>();

    // copy content to memory for speed
    io::MemoryByteStream input_stream(input_file.stream.read_to_eof());

    if (type == 6 || type == 7)
    {
        res::Image output_image(width, height);
        for (auto &c : output_image)
        {
            c.r = c.g = c.b = 0;
            c.a = 0xFF;
        }
        return output_image;
    }

    const auto output_info = get_format_info(type);
    const auto channels = res::pixel_format_to_bpp(output_info.fmt);

    bstr data;
    if (output_info.compressed)
        data = rle_decompress(input_stream, width, height, channels);
    else
        data = input_stream.read(width * height * channels);

    if (output_info.deinterleaved)
        data = interleave(data, width, height, channels);

    res::Image output_image(width, height, data, output_info.fmt);
    output_image.offset(x, y);
    output_image.crop(total_width, total_height);
    output_image.flip_vertically();
    return output_image;
}

static auto _ = dec::register_decoder<McgImageDecoder>("mebius/mcg");
