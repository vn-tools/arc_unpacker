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

#include "dec/kid/prt_image_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::kid;

static const bstr magic = "PRT\x00"_b;

bool PrtImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image PrtImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    const auto version = input_file.stream.read_le<u16>();

    if (version != 0x66 && version != 0x65)
        throw err::UnsupportedVersionError(version);

    const auto bit_depth = input_file.stream.read_le<u16>();
    const auto palette_offset = input_file.stream.read_le<u16>();
    const auto data_offset = input_file.stream.read_le<u16>();
    auto width = input_file.stream.read_le<u16>();
    auto height = input_file.stream.read_le<u16>();
    bool has_alpha = false;

    if (version == 0x66)
    {
        has_alpha = input_file.stream.read_le<u32>() != 0;
        const auto x = input_file.stream.read_le<u32>();
        const auto y = input_file.stream.read_le<u32>();
        const auto width2 = input_file.stream.read_le<u32>();
        const auto height2 = input_file.stream.read_le<u32>();
        if (width2) width = width2;
        if (height2) height = height2;
    }

    const auto stride = (((width * bit_depth / 8) + 3) / 4) * 4;

    res::Palette palette(
        bit_depth == 8 ? 256 : 0,
        input_file.stream.seek(palette_offset),
        res::PixelFormat::BGR888X);

    res::Image image(width, height);
    for (const auto y : algo::range(height))
    {
        input_file.stream.seek(data_offset + y * stride);
        const auto row = input_file.stream.read(stride);
        const auto *row_ptr = row.get<const u8>();
        for (const auto x : algo::range(width))
        {
            if (bit_depth == 8)
            {
                image.at(x, y) = palette[*row_ptr++];
            }
            else if (bit_depth == 24)
            {
                image.at(x, y)
                    = res::read_pixel<res::PixelFormat::BGR888>(row_ptr);
            }
            else if (bit_depth == 32)
            {
                image.at(x, y)
                    = res::read_pixel<res::PixelFormat::BGRA8888>(row_ptr);
            }
            else
            {
                throw err::UnsupportedBitDepthError(bit_depth);
            }
        }
    }

    image.flip_vertically();

    if (has_alpha)
    {
        image.apply_mask(res::Image(
            width, height, input_file.stream, res::PixelFormat::Gray8));
    }

    return image;
}

static auto _ = dec::register_decoder<PrtImageDecoder>("kid/prt");
