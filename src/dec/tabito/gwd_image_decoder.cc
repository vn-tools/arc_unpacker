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

static res::Image read_gwd_stream(io::BaseByteStream &input_stream)
{
    const auto base_pos = input_stream.pos();
    const auto stream_size = input_stream.read_le<u32>();
    if (input_stream.read(magic.size()) != magic)
        throw err::RecognitionError();
    const auto width = input_stream.read_be<u16>();
    const auto height = input_stream.read_be<u16>();
    const auto depth = input_stream.read<u8>();

    init_transform_table();

    bstr decoded_row(width);
    io::MsbBitStream bit_stream(input_stream);

    if (depth == 8)
    {
        bstr data(width * height);
        u8 *data_ptr = data.get<u8>();
        for (const auto y : algo::range(height))
        {
            decode_row(bit_stream, decoded_row);
            transform_row(decoded_row);
            for (const auto x : algo::range(width))
                *data_ptr++ = decoded_row[x];
        }
        return res::Image(width, height, data, res::PixelFormat::Gray8);
    }

    if (depth == 24)
    {
        bstr data(width * height * 3);
        for (const auto y : algo::range(height))
        for (const auto channel : algo::range(3))
        {
            decode_row(bit_stream, decoded_row);
            transform_row(decoded_row);
            u8 *data_ptr = data.get<u8>() + y * width * 3 + channel;
            for (const auto x : algo::range(width))
            {
                *data_ptr = decoded_row[x];
                data_ptr += 3;
            }
        }
        return res::Image(width, height, data, res::PixelFormat::BGR888);
    }

    throw err::UnsupportedBitDepthError(depth);
}

res::Image GwdImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    auto image = read_gwd_stream(input_file.stream.seek(0));
    if (input_file.stream.read<u8>())
    {
        image.apply_mask(read_gwd_stream(input_file.stream));
        for (auto &c : image)
            c.a = 255 - c.a;
    }
    return image;
}

static auto _ = dec::register_decoder<GwdImageDecoder>("tabito/gwd");
