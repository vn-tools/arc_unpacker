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

#include "dec/bgi/cbg/cbg2_decoder.h"
#include <array>
#include "algo/range.h"
#include "dec/bgi/cbg/cbg_common.h"
#include "err.h"
#include "io/memory_byte_stream.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::bgi::cbg;

static const int tree1_size = 0x10;
static const int tree2_size = 0xB0;
static const int block_dim = 8;
static const int block_dim2 = block_dim * block_dim;

static int jpeg_zigzag_order[block_dim2] =
{
    0,  1,  8,  16, 9,  2,  3,  10,
    17, 24, 32, 25, 18, 11, 4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13, 6,  7,  14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

namespace
{
    using FloatTable = std::array<float, block_dim2>;
    using FloatTablePair = std::array<FloatTable, 2>;
    using FloatTableTriplet = std::array<FloatTable, 3>;
}

static FloatTablePair read_ac_mul_pair(const bstr &input)
{
    static const FloatTable dc_table =
    {
        1.0f,      1.38704f,  1.30656f,  1.17588f,
        1.0f,      0.785695f, 0.541196f, 0.275899f,
        1.38704f,  1.92388f,  1.81225f,  1.63099f,
        1.38704f,  1.08979f,  0.750661f, 0.382683f,
        1.30656f,  1.81225f,  1.70711f,  1.53636f,
        1.30656f,  1.02656f,  0.707107f, 0.36048f,
        1.17588f,  1.63099f,  1.53636f,  1.38268f,
        1.17588f,  0.92388f,  0.636379f, 0.324423f,
        1.0f,      1.38704f,  1.30656f,  1.17588f,
        1.0f,      0.785695f, 0.541196f, 0.275899f,
        0.785695f, 1.08979f,  1.02656f,  0.92388f,
        0.785695f, 0.617317f, 0.425215f, 0.216773f,
        0.541196f, 0.750661f, 0.707107f, 0.636379f,
        0.541196f, 0.425215f, 0.292893f, 0.149316f,
        0.275899f, 0.382683f, 0.36048f,  0.324423f,
        0.275899f, 0.216773f, 0.149316f, 0.0761205f
    };

    FloatTablePair ac_mul_pair;
    io::MemoryByteStream input_stream(input);
    for (const auto i : algo::range(ac_mul_pair.size()))
        for (const auto j : algo::range(dc_table.size()))
            ac_mul_pair[i][j] = input_stream.read<u8>() * dc_table[j];
    return ac_mul_pair;
}

static s16 jpeg_ftoi(float result)
{
    int a = 0x80 + ((static_cast<int>(result)) >> 3);
    if (a < 0)
        return 0;
    if (a < 0xFF)
        return static_cast<s16>(a);
    if (a < 0x180)
        return 0xFF;
    return 0;
}

static void jpeg_dct_float(
    FloatTable &output, const u16 *ac, const FloatTable &ac_mul)
{
    s16 inptr[block_dim2];
    float dv[block_dim2];
    float tp[block_dim2];
    float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    float tmp10, tmp11, tmp12, tmp13;
    float z5, z10, z11, z12, z13;

    for (const auto i : algo::range(block_dim2))
    {
        inptr[i] = ac[i];
        dv[i] = ac_mul[i];
    }

    for (const auto i : algo::range(block_dim))
    {
        if (!inptr[8 + i] && !inptr[16 + i]
            && !inptr[24 + i] && !inptr[32 + i]
            && !inptr[40 + i] && !inptr[48 + i]
            && !inptr[56 + i])
        {
            tmp0 = inptr[i] * dv[i];
            tp[i] = tmp0;
            tp[8 + i] = tmp0;
            tp[16 + i] = tmp0;
            tp[24 + i] = tmp0;
            tp[32 + i] = tmp0;
            tp[40 + i] = tmp0;
            tp[48 + i] = tmp0;
            tp[56 + i] = tmp0;
            continue;
        }

        tmp0 = inptr[i] * dv[i];
        tmp1 = inptr[16 + i] * dv[16 + i];
        tmp2 = inptr[32 + i] * dv[32 + i];
        tmp3 = inptr[48 + i] * dv[48 + i];
        tmp10 = tmp0 + tmp2;
        tmp11 = tmp0 - tmp2;
        tmp13 = tmp1 + tmp3;
        tmp12 = (tmp1 - tmp3) * 1.414213562f - tmp13;
        tmp0 = tmp10 + tmp13;
        tmp3 = tmp10 - tmp13;
        tmp1 = tmp11 + tmp12;
        tmp2 = tmp11 - tmp12;
        tmp4 = inptr[8 + i] * dv[8 + i];
        tmp5 = inptr[24 + i] * dv[24 + i];
        tmp6 = inptr[40 + i] * dv[40 + i];
        tmp7 = inptr[56 + i] * dv[56 + i];
        z13 = tmp6 + tmp5;
        z10 = tmp6 - tmp5;
        z11 = tmp4 + tmp7;
        z12 = tmp4 - tmp7;

        tmp7 = z11 + z13;
        tmp11 = (z11 - z13) * 1.414213562f;
        z5 = (z10 + z12) * 1.847759065f;
        tmp10 = z12 * 1.082392200f - z5;
        tmp12 = z10 * (-2.613125930f) + z5;

        tmp6 = tmp12 - tmp7;
        tmp5 = tmp11 - tmp6;
        tmp4 = tmp10 + tmp5;

        tp[i] = tmp0 + tmp7;
        tp[56 + i] = tmp0 - tmp7;
        tp[8 + i] = tmp1 + tmp6;
        tp[48 + i] = tmp1 - tmp6;
        tp[16 + i] = tmp2 + tmp5;
        tp[40 + i] = tmp2 - tmp5;
        tp[32 + i] = tmp3 + tmp4;
        tp[24 + i] = tmp3 - tmp4;
    }

    for (const auto i : algo::range(block_dim))
    {
        z5 = tp[i * block_dim];
        tmp10 = z5 + tp[block_dim * i + 4];
        tmp11 = z5 - tp[block_dim * i + 4];

        tmp13 = tp[block_dim * i + 2] + tp[block_dim * i + 6];
        tmp12 = (tp[block_dim * i + 2] - tp[block_dim * i + 6])
            * 1.414213562f - tmp13;

        tmp0 = tmp10 + tmp13;
        tmp3 = tmp10 - tmp13;
        tmp1 = tmp11 + tmp12;
        tmp2 = tmp11 - tmp12;

        z13 = tp[block_dim * i + 5] + tp[block_dim * i + 3];
        z10 = tp[block_dim * i + 5] - tp[block_dim * i + 3];
        z11 = tp[block_dim * i + 1] + tp[block_dim * i + 7];
        z12 = tp[block_dim * i + 1] - tp[block_dim * i + 7];

        tmp7 = z11 + z13;
        tmp11 = (z11 - z13) * 1.414213562f;

        z5 = (z10 + z12) * 1.847759065f;
        tmp10 = z5 - z12 * 1.082392200f;
        tmp12 = z5 - z10 * 2.613125930f;

        tmp6 = tmp12 - tmp7;
        tmp5 = tmp11 - tmp6;
        tmp4 = tmp10 - tmp5;

        output[i * block_dim] = jpeg_ftoi(tmp0 + tmp7);
        output[i * block_dim + 7] = jpeg_ftoi(tmp0 - tmp7);
        output[i * block_dim + 1] = jpeg_ftoi(tmp1 + tmp6);
        output[i * block_dim + 6] = jpeg_ftoi(tmp1 - tmp6);
        output[i * block_dim + 2] = jpeg_ftoi(tmp2 + tmp5);
        output[i * block_dim + 5] = jpeg_ftoi(tmp2 - tmp5);
        output[i * block_dim + 3] = jpeg_ftoi(tmp3 + tmp4);
        output[i * block_dim + 4] = jpeg_ftoi(tmp3 - tmp4);
    }
}

static u8 get_component(float value)
{
    return std::max(0.0f, std::min(255.0f, value));
}

static std::vector<u16> decompress_block(
    size_t output_size,
    const bstr &input,
    const Tree &tree1,
    const Tree &tree2)
{
    std::vector<u16> color_info(output_size, 0);
    io::MsbBitStream bit_stream(input);

    int init_value = 0;
    for (const auto i : algo::range(0, output_size, block_dim2))
    {
        const auto size = tree1.get_leaf(bit_stream);
        if (size)
        {
            int value = bit_stream.read(size);
            if (((1 << (size - 1)) & value) == 0)
                value = (0xFFFFFFFF << size) | (value + 1);
            init_value += value;
        }
        color_info.at(i) = init_value & 0xFFFF;
    }

    // align to regular byte
    bit_stream.read((8 - (bit_stream.pos() & 7)) & 7);

    for (const auto i : algo::range(0, output_size, block_dim2))
    {
        auto index = 1;
        while (index < block_dim2)
        {
            auto size = tree2.get_leaf(bit_stream);
            if (!size)
                break;
            if (size < 0xF)
            {
                index += 0x10;
            }
            else
            {
                index += size & 0xF;
                if (index >= block_dim2)
                    break;
                size >>= 4;
                if (size)
                {
                    int value = bit_stream.read(size);
                    if (((1 << (size - 1)) & value) == 0 && size != 0)
                        value = (0xFFFFFFFF << size) | (value + 1);
                    color_info.at(i + jpeg_zigzag_order[index]) = value;
                }
                index++;
            }
        }
    }

    return color_info;
}

static void process_24bit_block(
    const std::vector<u16> &color_info,
    const FloatTablePair &ac_mul_pair,
    size_t width,
    u8 *rgb_out)
{
    FloatTableTriplet yuv_in;
    for (const auto i : algo::range(width / block_dim))
    {
        for (const auto channel : algo::range(3))
        {
            jpeg_dct_float(
                yuv_in[channel],
                &color_info[i * block_dim2 + channel * width * block_dim],
                ac_mul_pair[channel > 0]);
        }

        for (const auto y : algo::range(block_dim))
        for (const auto x : algo::range(block_dim))
        {
            const auto offset = y * block_dim + x;
            const auto cy = yuv_in[0][offset];
            const auto cb = yuv_in[1][offset];
            const auto cr = yuv_in[2][offset];

            const auto r = cy + 1.402f * cr - 178.956f;
            const auto g
                = cy + 44.04992f - 0.34414f * cb + 91.90992f - 0.71414f * cr;
            const auto b = cy + 1.772f * cb - 226.316f;

            u8 *rgb_ptr = &rgb_out[(y * width + x) * 4];
            rgb_ptr[0] = get_component(b);
            rgb_ptr[1] = get_component(g);
            rgb_ptr[2] = get_component(r);
        }
        rgb_out += 4 * block_dim;
    }
}

static void process_8bit_block(
    const std::vector<u16> &color_info,
    const FloatTablePair &ac_mul_pair,
    size_t width,
    u8 *rgb_out)
{
    for (const auto i : algo::range(width / block_dim))
    {
        FloatTable color_data;
        jpeg_dct_float(color_data, &color_info[i * block_dim2], ac_mul_pair[0]);
        for (const auto y : algo::range(block_dim))
        for (const auto x : algo::range(block_dim))
        {
            u8 *rgb_ptr = &rgb_out[(i * block_dim + (y * width + x)) * 4];
            rgb_ptr[0] = color_data[y * block_dim + x];
            rgb_ptr[1] = color_data[y * block_dim + x];
            rgb_ptr[2] = color_data[y * block_dim + x];
        }
    }
}

static void process_alpha(
    bstr &output, io::BaseByteStream &input_stream, int width)
{
    auto output_start = output.get<u8>();
    auto output_end = output_start + output.size();
    auto output_ptr = output_start + 3;

    u8 mark;
    u8 current_bit = 0;
    while (input_stream.left())
    {
        if (!current_bit)
        {
            current_bit = 8;
            mark = input_stream.read<u8>();
        }

        --current_bit;
        if (mark & (1 << (7 - current_bit)))
        {
            int tmp = input_stream.read_le<u16>();

            int look_behind_pos = tmp & 0x3F;
            if (look_behind_pos > 0x1F)
                look_behind_pos |= 0xFFFFFFC0;
            int count = ((tmp >> 9) & 0x7F) + 3;
            int y = (tmp >> 6) & 7;
            if (y != 0)
                y |= 0xFFFFFFF8;

            u8 *look_behind = &output_ptr[(look_behind_pos + y * width) * 4];
            if (look_behind < output_start || look_behind >= output_end)
                return;

            for (const auto i : algo::range(count))
            {
                *output_ptr = *look_behind;
                look_behind += 4;
                output_ptr += 4;
            }
        }
        else
        {
            *output_ptr = input_stream.read<u8>();
            output_ptr += 4;
        }
    }
}

std::unique_ptr<res::Image> Cbg2Decoder::decode(
    io::BaseByteStream &input_stream) const
{
    const size_t width = input_stream.read_le<u16>();
    const size_t height = input_stream.read_le<u16>();
    const size_t depth = input_stream.read_le<u32>();
    const size_t channels = depth >> 3;
    input_stream.skip(12);

    const auto decrypted_data = read_decrypted_data(input_stream);
    const auto ac_mul_pair = read_ac_mul_pair(decrypted_data);
    io::MemoryByteStream raw_stream(input_stream);

    const auto pad_width
        = width + ((block_dim - (width % block_dim)) % block_dim);
    const auto pad_height
        = height + ((block_dim - (height % block_dim)) % block_dim);
    const auto block_count = pad_height / block_dim;

    const auto tree1
        = build_tree(read_freq_table(raw_stream, tree1_size), true);
    const auto tree2
        = build_tree(read_freq_table(raw_stream, tree2_size), true);

    const auto block_offsets = std::make_unique<u32[]>(block_count + 1);
    for (const auto i : algo::range(block_count + 1))
        block_offsets[i] = raw_stream.read_le<u32>();

    bstr bmp_data(pad_width * pad_height * 4);
    for (const auto i : algo::range(bmp_data.size()))
        bmp_data.get<u8>()[i] = 0xFF;

    for (const auto i : algo::range(block_count))
    {
        raw_stream.seek(block_offsets[i]);
        raw_stream.skip((pad_width + block_dim2 - 1) / block_dim2);
        const auto block_size_orig = read_variable_data(raw_stream);
        int block_size_comp = block_offsets[i + 1] - raw_stream.pos();
        if (block_size_comp < 0)
            block_size_comp = raw_stream.size() - raw_stream.pos();
        const auto expected_width
            = pad_width * block_dim * (depth == 8 ? 1 : 3);
        if (expected_width != block_size_orig)
            throw err::BadDataSizeError();

        const auto block_data = raw_stream.read(block_size_comp);
        const auto color_info = decompress_block(
            block_size_orig, block_data, tree1, tree2);

        if (channels == 3 || channels == 4)
        {
            process_24bit_block(
                color_info,
                ac_mul_pair,
                pad_width,
                &bmp_data.get<u8>()[pad_width * block_dim * 4 * i]);
        }
        else if (channels == 1)
        {
            process_8bit_block(
                color_info,
                ac_mul_pair,
                pad_width,
                &bmp_data.get<u8>()[pad_width * block_dim * 4 * i]);
        }
        else
            throw err::UnsupportedChannelCountError(channels);
    }

    if (channels == 4)
    {
        raw_stream.seek(block_offsets[block_count]);
        if (raw_stream.read_le<u32>() == 1)
            process_alpha(bmp_data, raw_stream, pad_width);
    }

    auto image = std::make_unique<res::Image>(
        pad_width, pad_height, bmp_data, res::PixelFormat::BGRA8888);
    image->crop(width, height);
    return image;
}
