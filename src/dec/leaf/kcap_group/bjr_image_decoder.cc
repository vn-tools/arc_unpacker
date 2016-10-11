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

#include "dec/leaf/kcap_group/bjr_image_decoder.h"
#include "algo/range.h"
#include "dec/microsoft/bmp_image_decoder.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "BM"_b;

bool BjrImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("bjr");
}

res::Image BjrImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto bmp_image_decoder = dec::microsoft::BmpImageDecoder();
    const auto input = bmp_image_decoder.decode(logger, input_file);

    const auto name = input_file.path.name();
    u32 key1 = 0x10000;
    u32 key2 = 0;
    u32 key3 = 0;
    for (const auto i : algo::range(name.size()))
    {
        key1 -= name[i];
        key2 += name[i];
        key3 ^= name[i];
    }

    res::Image output(input.width(), input.height());
    for (const auto y : algo::range(output.height()))
    {
        key3 += 7;

        const auto src_line = &input.at(0, key3 % output.height());
        const auto dst_line = &output.at(0, y);
        for (const auto x : algo::range(output.width()))
        {
            if (x & 1)
            {
                dst_line[x].b = 0x10000 - key2 + src_line[x].b;
                dst_line[x].g = 0x100FF - key1 - src_line[x].g;
                dst_line[x].r = 0x10000 - key2 + src_line[x].r;
            }
            else
            {
                dst_line[x].b = 0x100FF - key1 - src_line[x].b;
                dst_line[x].g = 0x10000 - key2 + src_line[x].g;
                dst_line[x].r = 0x100FF - key1 - src_line[x].r;
            }
            dst_line[x].a = 0xFF;
        }
    }

    return output;
}

static auto _ = dec::register_decoder<BjrImageDecoder>("leaf/bjr");
