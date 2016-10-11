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

#include "dec/bgi/cbg/cbg1_decoder.h"
#include "algo/range.h"
#include "dec/bgi/cbg/cbg_common.h"
#include "err.h"
#include "io/memory_byte_stream.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::bgi::cbg;

static bstr decompress_huffman(
    io::BaseBitStream &bit_stream, const Tree &tree, size_t output_size)
{
    bstr output(output_size);
    for (const auto i : algo::range(output.size()))
        output[i] = tree.get_leaf(bit_stream);
    return output;
}

static bstr decompress_rle(bstr &input, size_t output_size)
{
    io::MemoryByteStream input_stream(input);

    bstr output(output_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.get<const u8>() + output.size();

    bool zero_flag = false;
    while (input_stream.left())
    {
        size_t size = read_variable_data(input_stream);
        if (output_ptr + size >= output_end)
            size = output_end - output_ptr;

        if (zero_flag)
        {
            for (const auto i : algo::range(size))
                *output_ptr++ = 0;
        }
        else
        {
            for (const auto i : algo::range(size))
                *output_ptr++ = input_stream.read<u8>();
        }
        zero_flag = !zero_flag;
    }

    return output;
}

static void transform_colors(bstr &input, u16 width, u16 height, u16 bpp)
{
    u16 channels = bpp >> 3;

    u8 *input_ptr = input.get<u8>();
    u8 *left = &input_ptr[- channels];
    u8 *above = &input_ptr[- width * channels];

    // ignore 0,0
    input_ptr += channels;
    above += channels;
    left += channels;

    // add left to first row
    for (const auto x : algo::range(1, width))
    {
        for (const auto i : algo::range(channels))
        {
            *input_ptr += input_ptr[-channels];
            input_ptr++;
            above++;
            left++;
        }
    }

    // add left and top to all other pixels
    for (const auto y : algo::range(1, height))
    {
        for (const auto i : algo::range(channels))
        {
            *input_ptr += *above;
            input_ptr++;
            above++;
            left++;
        }

        for (const auto x : algo::range(1, width))
        {
            for (const auto i : algo::range(channels))
            {
                *input_ptr += (*left + *above) >> 1;
                input_ptr++;
                above++;
                left++;
            }
        }
    }
}

static res::PixelFormat bpp_to_pixel_format(int bpp)
{
    switch (bpp)
    {
        case 8:
            return res::PixelFormat::Gray8;
        case 24:
            return res::PixelFormat::BGR888;
        case 32:
            return res::PixelFormat::BGRA8888;
    }
    throw err::UnsupportedBitDepthError(bpp);
}

std::unique_ptr<res::Image> Cbg1Decoder::decode(
    io::BaseByteStream &input_stream) const
{
    const auto width = input_stream.read_le<u16>();
    const auto height = input_stream.read_le<u16>();
    const auto bpp = input_stream.read_le<u32>();
    input_stream.skip(8);

    const auto huffman_size = input_stream.read_le<u32>();
    io::MemoryByteStream decrypted_stream(read_decrypted_data(input_stream));
    const auto raw_data = input_stream.read_to_eof();

    const auto freq_table = read_freq_table(decrypted_stream, 256);
    const auto tree = build_tree(freq_table, false);

    io::MsbBitStream bit_stream(raw_data);
    auto output = decompress_huffman(bit_stream, tree, huffman_size);
    auto pixel_data = decompress_rle(output, width * height * (bpp >> 3));
    transform_colors(pixel_data, width, height, bpp);

    const auto format = bpp_to_pixel_format(bpp);
    return std::make_unique<res::Image>(width, height, pixel_data, format);
}
