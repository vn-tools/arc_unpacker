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

#include "dec/complets/gd_image_decoder.h"
#include "algo/format.h"
#include "algo/pack/lzss.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "err.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::complets;

static const bstr magic2 = "GD2"_b;
static const bstr magic3 = "GD3"_b;

namespace
{
    enum class CompressionType : u8
    {
        Uncompressed = 0x62,
        Lzss         = 0x6C,
        Custom       = 0x70,
    };
}

static bstr decompress_lzss(
    io::BaseBitStream &bit_stream,
    const size_t width,
    const size_t height,
    const size_t channels)
{
    std::vector<u8> dict(1 << 16, 0);
    auto dict_ptr = algo::make_cyclic_ptr(dict.data(), dict.size()) + 1;

    bstr output(width * height * channels);
    auto output_ptr = algo::make_ptr(output);
    while (output_ptr.left() && bit_stream.left())
    {
        if (bit_stream.read(1))
        {
            if (bit_stream.left() < 8)
                break;
            const auto b = bit_stream.read(8);
            *output_ptr++ = b;
            *dict_ptr++ = b;
        }
        else
        {
            if (bit_stream.left() < 20)
                break;
            auto look_behind_pos = bit_stream.read(16);
            auto repetitions = bit_stream.read(4) + 3;
            auto source_ptr
                = algo::make_cyclic_ptr(dict.data(), dict.size())
                + look_behind_pos;
            while (repetitions-- && output_ptr.left())
            {
                const auto b = *source_ptr++;
                *output_ptr++ = b;
                *dict_ptr++ = b;
            }
        }
    }
    return output;
}

static bstr decompress_custom(
    io::BaseBitStream &bit_stream,
    const size_t width,
    const size_t height,
    const size_t channels)
{
    bstr output(width * height * channels, 0xFF);
    auto ptr1 = algo::make_ptr(output);

    while (true)
    {
        auto skip = bit_stream.read(2);
        if (skip == 2)
        {
            skip = bit_stream.read(2) + 2;
        }
        else if (skip == 3)
        {
            int j = 3;
            while (bit_stream.read(1) && bit_stream.left())
                ++j;
            if (j >= 0x18)
                break;
            skip = ((1 << j) | bit_stream.read(j)) - 2;
        }

        ptr1 += 3 * skip;
        if (ptr1.left() < 3)
            throw err::BadDataOffsetError();
        ptr1[0] = bit_stream.read(8);
        ptr1[1] = bit_stream.read(8);
        ptr1[2] = bit_stream.read(8);
        if (bit_stream.read(1))
        {
            auto ptr2 = algo::make_ptr(output) + ptr1.pos();
            while (true)
            {
                const auto strategy = bit_stream.read(2);
                if (strategy == 0)
                {
                    if (!bit_stream.read(1))
                        break;
                    ptr2 += (width + (bit_stream.read(1) ? 2 : -2)) * channels;
                }
                else if (strategy == 1)
                    ptr2 += (width - 1) * channels;
                else if (strategy == 2)
                    ptr2 += width * channels;
                else if (strategy == 3)
                    ptr2 += (width + 1) * channels;
                if (ptr2.left() < 3)
                    throw err::BadDataOffsetError();
                ptr2[0] = ptr1[0];
                ptr2[1] = ptr1[1];
                ptr2[2] = ptr1[2];
            }
        }
        ptr1 += 3;
    }

    auto output_ptr = output.get<u8>();
    int buf[3] = {0, 0, 0};
    for (const auto y : algo::range(height))
    for (const auto x : algo::range(width))
    {
        const auto diff
            = output_ptr[0] != 0xFF
            || output_ptr[1] != 0xFF
            || output_ptr[2] != 0xFF;
        for (const auto c : algo::range(channels))
        {
            if (diff)
                buf[c] = *output_ptr++;
            else
                *output_ptr++ = buf[c];
        }
    }
    return output;
}

bool GdImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic2.size()) == magic2
        || input_file.stream.seek(0).read(magic3.size()) == magic3;
}

res::Image GdImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto version = input_file.stream.seek(2).read<u8>() - '0';
    const auto fmt = res::PixelFormat::BGR888;
    const auto channels = 3;
    const auto width = version == 2 ? 640 : 800;
    const auto height = version == 2 ? 480 : 600;
    const auto thumb_width = width / 10;
    const auto thumb_height = (height / 10) - 1;
    const auto compression_type = input_file.stream
        .seek(4 + channels * thumb_width * thumb_height)
        .read<CompressionType>();
    input_file.stream.skip(1);

    bstr output;

    if (compression_type == CompressionType::Uncompressed)
    {
        output = input_file.stream.read_to_eof();
    }
    else if (compression_type == CompressionType::Lzss)
    {
        io::MsbBitStream bit_stream(input_file.stream.read_to_eof());
        output = decompress_lzss(
            bit_stream, width, height, channels);
    }
    else if (compression_type == CompressionType::Custom)
    {
        io::MsbBitStream bit_stream(input_file.stream.read_to_eof());
        output = decompress_custom(
            bit_stream, width, height, channels);
    }
    else
    {
        throw err::NotSupportedError(
            algo::format("Unknown compression type: %x", compression_type));
    }

    return res::Image(width, height, output, fmt).flip_vertically();
}

static auto _ = dec::register_decoder<GdImageDecoder>("complets/gd");
