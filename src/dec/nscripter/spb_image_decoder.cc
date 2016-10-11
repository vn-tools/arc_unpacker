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

#include "dec/nscripter/spb_image_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "err.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::nscripter;

static res::Image decode_image(
    io::BaseBitStream &input_stream, const size_t width, const size_t height)
{
    res::Image output(width, height);
    for (auto &c : output)
        c.a = 0xFF;

    for (const auto rgb : algo::range(3))
    {
        bstr channel_data(width * height + width);
        auto channel_ptr = algo::make_ptr(channel_data);

        u8 ch = input_stream.read(8);
        *channel_ptr++ = ch;

        while (channel_ptr.pos() < width * height)
        {
            const auto t = input_stream.read(3);
            if (!t)
            {
                for (const auto i : algo::range(4))
                    *channel_ptr++ = ch;
                continue;
            }

            const auto mask = t == 7 ? input_stream.read(1) + 1 : t + 2;
            for (const auto i : algo::range(4))
            {
                if (mask == 8)
                {
                    ch = input_stream.read(8);
                }
                else
                {
                    const auto tmp = input_stream.read(mask);
                    if (tmp & 1)
                        ch += (tmp >> 1) + 1;
                    else
                        ch -= (tmp >> 1);
                }
                *channel_ptr++ = ch;
            }
        }

        channel_ptr = algo::make_ptr(channel_data);
        for (const auto y : algo::range(height))
        for (const auto x : algo::range(width))
        {
            if (y & 1)
                output.at(width - 1 - x, y)[rgb] = *channel_ptr++;
            else
                output.at(x, y)[rgb] = *channel_ptr++;
        }
    }

    return output;
}

bool SpbImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("bmp"))
        return false;
    const auto width = input_file.stream.read_be<u16>();
    const auto height = input_file.stream.read_be<u16>();
    if (height == 0 || width == 0)
        return false;
    if (width > 5000 || height > 5000)
        return false;
    return true;
}

res::Image SpbImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto width = input_file.stream.read_be<u16>();
    const auto height = input_file.stream.read_be<u16>();
    io::MsbBitStream bit_stream(input_file.stream.read_to_eof());
    return decode_image(bit_stream, width, height);
}

static auto _ = dec::register_decoder<SpbImageDecoder>("nscripter/spb");
