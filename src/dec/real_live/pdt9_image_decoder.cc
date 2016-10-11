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

// Part of AVG32 engine, that later evolved into RealLive engine

#include "dec/real_live/pdt9_image_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::real_live;

bool Pdt9ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("pdt")
        && input_file.stream.seek(0).read(1) == "9"_b;
}

res::Image Pdt9ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(3);
    const auto canvas_x = input_file.stream.read_le<u16>();
    const auto canvas_y = input_file.stream.read_le<u16>();
    const auto canvas_width = input_file.stream.read_le<u16>() + 1;
    const auto canvas_height = input_file.stream.read_le<u16>() + 1;

    res::Palette palette(256, input_file.stream, res::PixelFormat::BGR888X);

    std::vector<size_t> look_behinds;
    for (const auto i : algo::range(16))
        look_behinds.push_back(input_file.stream.read_le<s16>());

    const auto total_width = (canvas_width - canvas_x);
    const auto total_height = (canvas_height - canvas_y);
    bstr output(total_width * total_height + total_width);
    auto output_ptr = algo::make_ptr(output) + total_width;

    auto control = 0;
    auto mask = 0;
    while (output_ptr.left() && input_file.stream.left())
    {
        mask >>= 1;
        if (!mask)
        {
            mask = 0x80;
            control = input_file.stream.read<u8>();
        }
        if (control & mask)
        {
            const auto tmp = input_file.stream.read<u8>();
            const auto look_behind = look_behinds.at(tmp >> 4);
            auto repetitions = (tmp & 0xF) + 2;
            while (repetitions-- && output_ptr.left())
            {
                *output_ptr = output_ptr.pos() >= look_behind
                    ? output_ptr[-look_behind]
                    : 0;
                output_ptr++;
            }
        }
        else
        {
            *output_ptr++ = input_file.stream.read<u8>();
        }
    }
    output = output.substr(total_width);
    output.resize(total_width * total_height);

    return res::Image(total_width, total_height, output, palette);
}

static auto _ = dec::register_decoder<Pdt9ImageDecoder>("real-live/pdt9");
