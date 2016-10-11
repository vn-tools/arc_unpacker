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

#include "dec/microsoft/dxt/dxt_decoders.h"
#include "algo/range.h"

using namespace au;

static std::unique_ptr<res::Image> create_image(
    const size_t width, const size_t height)
{
    return std::make_unique<res::Image>((width + 3) & ~3, (height + 3) & ~3);
}

static void decode_dxt1_block(
    io::BaseByteStream &input_stream, res::Pixel output_colors[4][4])
{
    res::Pixel colors[4];
    const auto tmp = input_stream.read(4);
    const auto *tmp_ptr = tmp.get<const u8>();
    colors[0] = res::read_pixel<res::PixelFormat::BGR565>(tmp_ptr);
    colors[1] = res::read_pixel<res::PixelFormat::BGR565>(tmp_ptr);
    const auto transparent
        = colors[0].b <= colors[1].b
        && colors[0].g <= colors[1].g
        && colors[0].r <= colors[1].r
        && colors[0].a <= colors[1].a;

    for (const auto i : algo::range(4))
    {
        if (!transparent)
        {
            colors[2][i] = ((colors[0][i] << 1) + colors[1][i]) / 3;
            colors[3][i] = ((colors[1][i] << 1) + colors[0][i]) / 3;
        }
        else
        {
            colors[2][i] = (colors[0][i] + colors[1][i]) >> 1;
            colors[3][i] = 0;
        }
    }

    auto lookup = input_stream.read_le<u32>();
    for (const auto y : algo::range(4))
    for (const auto x : algo::range(4))
    {
        const auto index = lookup & 3;
        output_colors[y][x] = colors[index];
        lookup >>= 2;
    }
}

static void decode_dxt5_block(
    io::BaseByteStream &input_stream, u8 output_alpha[4][4])
{
    u8 alpha[8];
    alpha[0] = input_stream.read<u8>();
    alpha[1] = input_stream.read<u8>();

    if (alpha[0] > alpha[1])
    {
        for (const auto i : algo::range(2, 8))
            alpha[i] = ((8. - i) * alpha[0] + ((i - 1.) * alpha[1])) / 7.;
    }
    else
    {
        for (const auto i : algo::range(2, 6))
            alpha[i] = ((6. - i) * alpha[0] + ((i - 1.) * alpha[1])) / 5.;
        alpha[6] = 0;
        alpha[7] = 255;
    }

    for (const auto i : algo::range(2))
    {
        u32 lookup = input_stream.read<u8>();
        lookup |= input_stream.read<u8>() << 8;
        lookup |= input_stream.read<u8>() << 16;
        for (const auto j : algo::range(8))
        {
            const auto index = lookup & 7;
            const auto pos = i * 8 + j;
            const auto x = pos % 4;
            const auto y = pos / 4;
            lookup >>= 3;
            output_alpha[y][x] = alpha[index];
        }
    }
}

std::unique_ptr<res::Image> dec::microsoft::dxt::decode_dxt1(
    io::BaseByteStream &input_stream, size_t width, size_t height)
{
    auto image = create_image(width, height);
    for (const auto block_y : algo::range(0, height, 4))
    for (const auto block_x : algo::range(0, width, 4))
    {
        res::Pixel colors[4][4];
        decode_dxt1_block(input_stream, colors);
        for (const auto y : algo::range(4))
        for (const auto x : algo::range(4))
            image->at(block_x + x, block_y + y) = colors[y][x];
    }
    return image;
}

std::unique_ptr<res::Image> dec::microsoft::dxt::decode_dxt3(
    io::BaseByteStream &input_stream, size_t width, size_t height)
{
    auto image = create_image(width, height);
    for (const auto block_y : algo::range(0, height, 4))
    for (const auto block_x : algo::range(0, width, 4))
    {
        u8 alpha[4][4];
        for (const auto y : algo::range(4))
        for (const auto x : algo::range(0, 4, 2))
        {
            const auto b = input_stream.read<u8>();
            alpha[y][x + 0] = b & 0xF0;
            alpha[y][x + 1] = (b & 0x0F) << 4;
        }

        res::Pixel colors[4][4];
        decode_dxt1_block(input_stream, colors);
        for (const auto y : algo::range(4))
        for (const auto x : algo::range(4))
        {
            colors[y][x].a = alpha[y][x];
            image->at(block_x + x, block_y + y) = colors[y][x];
        }
    }
    return image;
}

std::unique_ptr<res::Image> dec::microsoft::dxt::decode_dxt5(
    io::BaseByteStream &input_stream, size_t width, size_t height)
{
    auto image = create_image(width, height);
    for (const auto block_y : algo::range(0, height, 4))
    for (const auto block_x : algo::range(0, width, 4))
    {
        u8 alpha[4][4];
        decode_dxt5_block(input_stream, alpha);

        res::Pixel colors[4][4];
        decode_dxt1_block(input_stream, colors);
        for (const auto y : algo::range(4))
        for (const auto x : algo::range(4))
        {
            colors[y][x].a = alpha[y][x];
            image->at(block_x + x, block_y + y) = colors[y][x];
        }
    }
    return image;
}
