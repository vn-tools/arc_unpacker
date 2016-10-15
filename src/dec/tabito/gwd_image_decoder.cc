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

#include "dec/tabito/gwd_image_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "err.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::tabito;

static const bstr magic = "GWD"_b;
static u8 transform_table[256][256];

bool GwdImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(4).read(magic.size()) == magic;
}

namespace
{
    struct bit_reader final
    {
        u32 bits_available;
        u8 current_byte;
    };
}

static void init_transform_table()
{
    for (const auto i : algo::range(256))
    for (const auto j : algo::range(256))
    {
        const u8 tmp = j >= 0x80 ? 0xFF - j : j;
        auto result = tmp << 1;
        if (result >= i)
        {
            result = i & 1
                ? tmp + ((i + 1) >> 1)
                : tmp - (i >> 1);
        }
        else
        {
            result = i;
        }
        transform_table[i][j] = j >= 0x80
            ? 0xFF - result
            : result;
    }
}

static u32 read_gamma_bits(io::BaseBitStream &input_stream)
{
    int num = 1;
    while (!input_stream.read(1))
        ++num;
    return input_stream.read(num) + (1 << num) - 2;
}

static void decode_row(io::BaseBitStream &input_stream, bstr &output_row)
{
    if (!output_row.size())
        return;
    auto output_row_ptr = algo::make_ptr(output_row);
    while (output_row_ptr.left())
    {
        const auto bits = input_stream.read(3);
        const auto size = read_gamma_bits(input_stream) + 1;
        if (size > output_row_ptr.left())
            throw err::BadDataSizeError();
        if (bits)
        {
            for (const auto i : algo::range(size))
                *output_row_ptr++ = input_stream.read(bits + 1);
        }
        else
        {
            for (const auto i : algo::range(size))
                *output_row_ptr++ = 0;
        }
    }
}

static void transform_row(bstr &row)
{
    for (const auto i : algo::range(1, row.size()))
        row[i] = transform_table[row[i]][row[i - 1]];
}

res::Image GwdImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(4 + magic.size());
    const auto width = input_file.stream.read_be<u16>();
    const auto height = input_file.stream.read_be<u16>();
    const auto depth = input_file.stream.read<u8>();
    const auto channels = depth / 8;

    res::Image image(width, height);
    for (const auto y : algo::range(height))
    for (const auto x : algo::range(width))
        image.at(x, y) = {0,0,0,0xFF};

    init_transform_table();

    bstr decoded_row(width);
    io::MsbBitStream bit_stream(input_file.stream);
    for (const auto y : algo::range(height))
    for (const auto channel : algo::range(channels))
    {
        decode_row(bit_stream, decoded_row);
        transform_row(decoded_row);

        for (const auto x : algo::range(width))
            image.at(x, y)[channel] = decoded_row[x];
    }

    return image;
}

static auto _ = dec::register_decoder<GwdImageDecoder>("tabito/gwd");
