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

#include "dec/pajamas/ep_image_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::pajamas;

static const auto magic = "EP"_b;

bool EpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image EpImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    input_file.stream.skip(1);
    const auto mode = input_file.stream.read<u8>();
    const auto color_type = input_file.stream.read_le<u32>();
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();

    const auto x = mode == 2 ? input_file.stream.read_le<u32>() : 0;
    const auto y = mode == 2 ? input_file.stream.read_le<u32>() : 0;

    if (color_type == 0 || color_type == 4)
        throw err::NotSupportedError("Paletted images are not supported");

    res::PixelFormat format;
    switch (color_type)
    {
        case 0: format = res::PixelFormat::Gray8; break;
        case 1: format = res::PixelFormat::BGR888; break;
        case 2: format = res::PixelFormat::BGRA8888; break;
        case 3: format = res::PixelFormat::BGR555X; break;
        case 4: format = res::PixelFormat::Gray8; break;
        default: throw err::NotSupportedError("Unknown color type");
    }

    std::unique_ptr<res::Palette> palette;
    if (format == res::PixelFormat::Gray8)
    {
        palette = std::make_unique<res::Palette>(
            256, input_file.stream, res::PixelFormat::BGR888);
    }

    const size_t offset_table[16] =
    {
        0,
        1,
        width,
        width + 1,
        2,
        width - 1,
        width * 2,
        3,
        (width + 1) * 2,
        width + 2,
        width * 2 + 1,
        width * 2 - 1,
        (width - 1) * 2,
        width - 2,
        width * 3,
        4,
    };

    bstr data(width * height * res::pixel_format_to_bpp(format));
    auto data_ptr = algo::make_ptr(data);
    while (data_ptr.left())
    {
        const auto flag = input_file.stream.read<u8>();
        if (flag & 0xF0)
        {
            const size_t count = flag & 8
                ? input_file.stream.read<u8>() + ((flag & 7) << 8)
                : flag & 7;
            if (data_ptr.left() < count)
                break;
            data_ptr.append_self(-offset_table[flag >> 4], count);
        }
        else
        {
            const auto count = std::min<size_t>(flag, data_ptr.left());
            for (const auto i : algo::range(count))
                *data_ptr++ = input_file.stream.read<u8>();
        }
    }

    const auto pixel_size = res::pixel_format_to_bpp(format);
    if (pixel_size > 1)
    {
        bstr unscrambled(data.size());
        data_ptr = algo::make_ptr(data);
        for (const auto p : algo::range(pixel_size))
        for (const auto y : algo::range(height))
        for (const auto x : algo::range(width))
        {
            unscrambled[(y * width * pixel_size + p) + x * pixel_size]
                = *data_ptr++;
        }
        data = unscrambled;
    }

    std::unique_ptr<res::Image> ret;
    if (palette)
        ret = std::make_unique<res::Image>(width, height, data, *palette);
    else
        ret = std::make_unique<res::Image>(width, height, data, format);
    ret->offset(x, y);
    return *ret;
}

static auto _
    = dec::register_decoder<EpImageDecoder>("pajamas/ep");
