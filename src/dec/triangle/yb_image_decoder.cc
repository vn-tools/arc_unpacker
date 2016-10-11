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

#include "dec/triangle/yb_image_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::triangle;

static const bstr magic = "YB"_b;

bool YbImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image YbImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto flags = input_file.stream.read<u8>();
    const auto channels = input_file.stream.read<u8>();
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();

    if (flags & 0x40)
        throw err::NotSupportedError("Flag 0x40 not implemented");

    const auto input = input_file.stream.read_to_eof();
    bstr output(width * height * channels);
    auto input_ptr = algo::make_ptr(input);
    auto output_ptr = algo::make_ptr(output);

    size_t p = 0;
    size_t n = 0;
    u16 control = 0;

    while (output_ptr.left() && input_ptr.left())
    {
        control <<= 1;
        if (!(control & 0x80))
            control = (*input_ptr++ << 8) | 0xFF;

        if (control >> 15)
        {
            if (!input_ptr.left())
                throw err::BadDataSizeError();
            const auto cmd1 = *input_ptr++;
            if (cmd1 & 0x80)
            {
                if (!input_ptr.left())
                    throw err::BadDataSizeError();
                const auto cmd2 = *input_ptr++;
                if (cmd1 & 0x40)
                {
                    p = cmd2 + ((cmd1 & 0x3F) << 8) + 1;
                    if (!input_ptr.left())
                        throw err::BadDataSizeError();
                    const auto cmd3 = *input_ptr++;
                    switch (cmd3)
                    {
                        case 0xFF:
                            n = 4096;
                            break;
                        case 0xFE:
                            n = 1024;
                            break;
                        case 0xFD:
                            n = 256;
                            break;
                        default:
                            n = cmd3 + 3;
                            break;
                    }
                }
                else
                {
                    p = (cmd2 >> 4) + ((cmd1 & 0x3F) << 4) + 1;
                    n = (cmd2 & 0xF) + 3;
                }
                output_ptr.append_self(-p, n);
            }
            else
            {
                if ((cmd1 & 3) == 3)
                {
                    n = (cmd1 >> 2) + 9;
                    output_ptr.append_from(input_ptr, n);
                }
                else
                {
                    p = (cmd1 >> 2) + 1;
                    n = (cmd1 & 3) + 2;
                    output_ptr.append_self(-p, n);
                }
            }
        }
        else
        {
            if (!input_ptr.left())
                throw err::BadDataSizeError();
            *output_ptr++ = *input_ptr++;
        }
    }

    if (flags & 0x80)
    {
        for (const auto i : algo::range(channels))
        for (const auto j : algo::range(channels + i, output.size(), channels))
            output[j] += output[j - channels];
    }

    res::PixelFormat fmt;
    if (channels == 4)
        fmt = res::PixelFormat::BGRA8888;
    else
        throw err::UnsupportedChannelCountError(channels);
    return res::Image(width, height, output, fmt);
}

static auto _ = dec::register_decoder<YbImageDecoder>("triangle/yb");
