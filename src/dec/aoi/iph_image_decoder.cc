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

#include "dec/aoi/iph_image_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::aoi;

static const bstr magic1 = "IPH\x00"_b;
static const bstr magic2 = "IPH\x20"_b;

static bstr decompress(
    io::BaseByteStream &input_stream,
    const size_t width,
    const size_t height,
    const size_t depth)
{
    bstr output(width * height * depth / 8);
    const auto stride = width * 2;
    bstr extra_line(stride);

    for (const auto y : algo::range(height))
    {
        const auto row = y * stride;
        auto control = input_stream.read<u8>();
        if (control)
        {
            auto output_ptr = algo::make_ptr(output) + row;
            u16 pixel = 0;
            while (output_ptr.left() >= 2)
            {
                control = input_stream.read<u8>();
                if (control == 0xFF)
                    break;
                if (control == 0xFE)
                {
                    auto repetitions = input_stream.read<u8>() + 1;
                    pixel = input_stream.read_le<u16>();
                    while (repetitions-- && output_ptr.left() >= 2)
                    {
                        *output_ptr++ = pixel & 0xFF;
                        *output_ptr++ = pixel >> 8;
                    }
                }
                else if (control < 0x80)
                {
                    const auto lo = input_stream.read<u8>();
                    *output_ptr++ = lo;
                    *output_ptr++ = control;
                    pixel = control << 8 | lo;
                }
                else
                {
                    control &= 0x7F;
                    const u8 r = (pixel & 0x7C00) >> 10;
                    const u8 g = (pixel & 0x3E0) >> 5;
                    const u8 b = pixel & 0x1F;
                    pixel = (b + control / 25 % 5 - 2)
                        | (g + control / 5 % 5 - 2) << 5
                        | (r + control % 5 - 2) << 10;
                    *output_ptr++ = pixel & 0xFF;
                    *output_ptr++ = pixel >> 8;
                }
            }
        }
        else
        {
            auto output_ptr = algo::make_ptr(output) + row;
            auto repetitions = width;
            while (repetitions-- && output_ptr.left() >= 2)
            {
                *output_ptr++ = input_stream.read<u8>();
                *output_ptr++ = input_stream.read<u8>();
            }
        }

        control = input_stream.read<u8>();
        if (control)
        {
            auto extra_line_ptr = algo::make_ptr(extra_line);
            while (extra_line_ptr.left())
            {
                control = input_stream.read<u8>();
                if (control == 0xFF)
                    break;
                if (control >= 0x80)
                {
                    auto repetitions = input_stream.read<u8>() + 1;
                    while (repetitions-- && extra_line_ptr.left())
                        *extra_line_ptr++ = control & 0x7F;
                }
                else
                {
                    *extra_line_ptr++ = control;
                }
            }
            auto output_ptr = algo::make_ptr(output) + row;
            for (const auto i : algo::range(width))
            {
                if (((32 >> i % 6) & extra_line.at(i / 6)) != 0)
                    output_ptr[1] |= 0x80;
                output_ptr += 2;
            }
        }
        else
        {
            input_stream.skip(1);
        }
    }
    return output;
}

bool IphImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return (input_file.stream.seek(8).read(magic1.size()) == magic1
        || input_file.stream.seek(8).read(magic2.size()) == magic2)
        && input_file.stream.seek(0x38).read(4) == "bmp "_b;
}

res::Image IphImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0x3C);
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    input_file.stream.seek(0x50);
    const auto depth = input_file.stream.read_le<u16>();
    const auto compressed = input_file.stream.read_le<u16>() != 0;
    input_file.stream.seek(0x58);
    const auto data = compressed
        ? decompress(input_file.stream, width, height, depth)
        : input_file.stream.read_to_eof();
    if (depth != 16)
        throw err::UnsupportedBitDepthError(depth);
    return res::Image(width, height, data, res::PixelFormat::BGR555X);
}

static auto _ = dec::register_decoder<IphImageDecoder>("aoi/iph");
