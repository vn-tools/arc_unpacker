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

#include "dec/lizsoft/sotes_image_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::lizsoft;

bool SotesImageDecoder::is_recognized_impl(io::File &input_file) const
{
    const auto a = input_file.stream.seek(0x438).read_le<u32>();
    const auto b = input_file.stream.seek(0x448).read_le<u32>();
    const auto c = input_file.stream.seek(0x450).read_le<u32>();
    return a - b == 0x2711 && c - b <= 0x80;
}

res::Image SotesImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto base = input_file.stream.seek(0x448).read_le<u32>();
    const auto pixel_data_offset
        = 0x458 + input_file.stream.seek(0x450).read_le<u32>() - base;

    const auto depth = input_file.stream.seek(0x430).read_le<u16>() - base;
    const auto tmp1 = input_file.stream.seek(0x440).read_le<u32>() - base;
    const auto width
        = input_file.stream.seek(4 + 4 * tmp1).read_le<u32>() - base;

    const auto tmp2 = input_file.stream.seek(0x18).read_le<u32>() - base;
    const auto height
        = input_file.stream.seek(0x420 + 4 * tmp2).read_le<u32>() - base;

    res::Palette palette(
        256,
        input_file.stream
            .seek(0x20)
            .read(256 * 4),
        res::PixelFormat::BGR888X);

    const auto data = input_file.stream
        .seek(pixel_data_offset)
        .read(width * height * (depth >> 3));

    std::unique_ptr<res::Image> image;
    if (depth == 8)
    {
        image = std::make_unique<res::Image>(width, height, data, palette);
    }
    else if (depth == 24)
    {
        image = std::make_unique<res::Image>(
            width, height, data, res::PixelFormat::BGR888);
    }
    else
    {
        throw err::UnsupportedBitDepthError(depth);
    }

    image->flip_vertically();
    return *image;
}

static auto _ = dec::register_decoder<SotesImageDecoder>("lizsoft/sotes");
