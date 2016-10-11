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

#include "dec/purple_software/jbp1.h"
#include <array>
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"
#include "io/msb_bit_stream.h"

using namespace au;

namespace
{
    struct BasicInfo final
    {
        size_t width;
        size_t height;
        size_t depth;
        u32 flags;
        uoff_t data_offset;
        size_t bit_pool_1_size;
        size_t bit_pool_2_size;
        size_t blocks_width;
        size_t blocks_height;
        size_t block_stride;
        size_t x_block_count;
        size_t y_block_count;
        size_t x_block_size;
        size_t y_block_size;
    };

    struct Tree final
    {
        Tree();
        std::array<u8, 0x100> base;
        std::array<u32, 0x400> neighbour;
        std::array<u32, 0x102> other;
        size_t root;
        size_t input_size;
    };

    class CustomBitStream final : public io::BaseBitStream
    {
    public:
        CustomBitStream(const bstr &str);
        u32 read(const size_t bits) override;
    };
}

Tree::Tree() : base(), neighbour(), other()
{
}

CustomBitStream::CustomBitStream(const bstr &input) : io::BaseBitStream(input)
{
}

u32 CustomBitStream::read(const size_t bits)
{
    u32 ret = 0;
    for (const auto i : algo::range(bits))
    {
        if (!bits_available)
        {
            buffer = input_stream->read<u8>();
            bits_available = 8;
        }
        ret <<= 1;
        ret |= (buffer & 1);
        buffer >>= 1;
        bits_available--;
    }
    return ret;
}

static Tree make_tree(const bstr &input, std::array<u32, 0x80> &freq)
{
    Tree ret;
    for (const auto i : algo::range(input.size()))
        ret.base.at(i) = input[i];

    static const size_t max = 2100000000;
    auto size = input.size();
    int c = ~size + 1;
    auto idx = size + 0x200;

    while (true)
    {
        int d = -1;
        int n = -1;

        {
            size_t x = max - 1;
            for (const auto i : algo::range(size))
            {
                if (freq[i] < x)
                {
                    n = i;
                    x = freq[i];
                }
            }
        }

        {
            size_t x = max - 1;
            for (const auto i : algo::range(size))
            {
                if ((i != n) && (freq[i] < x))
                {
                    d = i;
                    x = freq[i];
                }
            }
        }

        if (n < 0 || d < 0)
            break;

        ret.neighbour.at(idx - 0x200) = n;
        ret.neighbour.at(idx + 0) = d;
        idx++;

        ret.other[n] = size;
        ret.other[d] = c;
        freq[size] = freq[n] + freq[d];
        size++;
        c--;
        freq[n] = max;
        freq[d] = max;
    }

    ret.root = size - 1;
    ret.input_size = input.size();
    return ret;
}

static int read_from_tree(const Tree &tree, io::BaseBitStream &bit_stream)
{
    size_t ret = tree.root;
    while (ret >= tree.input_size)
        ret = tree.neighbour.at((bit_stream.read(1) << 9) + ret);
    return ret;
}

static void dct(
    std::array<s16, 64> &dct_table, const std::array<s16, 64> &quant)
{
    long a, b, c, d;
    long w, x, y, z;
    long s, t, u, v, n;

    auto lp1 = dct_table.data();
    auto lp2 = quant.data();

    for (const auto i : algo::range(8))
    {
        if (lp1[0x08] == 0 &&
            lp1[0x10] == 0 &&
            lp1[0x18] == 0 &&
            lp1[0x20] == 0 &&
            lp1[0x28] == 0 &&
            lp1[0x30] == 0 &&
            lp1[0x38] == 0)
        {
            lp1[0x00] =
            lp1[0x08] =
            lp1[0x10] =
            lp1[0x18] =
            lp1[0x20] =
            lp1[0x28] =
            lp1[0x30] =
            lp1[0x38] = lp1[0] * lp2[0];
        }

        else
        {
            c = lp2[0x10] * lp1[0x10];
            d = lp2[0x30] * lp1[0x30];
            x = ((c + d) * 35467) >> 16;
            c = ((c * 50159) >> 16) + x;
            d = ((d * -121094) >> 16) + x;
            a = lp1[0x00] * lp2[0x00];
            b = lp1[0x20] * lp2[0x20];
            w = a + b + c;
            x = a + b - c;
            y = a - b + d;
            z = a - b - d;

            c = lp1[0x38] * lp2[0x38];
            d = lp1[0x28] * lp2[0x28];
            a = lp1[0x18] * lp2[0x18];
            b = lp1[0x08] * lp2[0x08];
            n = ((a + b + c + d) * 77062) >> 16;

            u = n
                + ((c * 19571) >> 16)
                + (((c + a) * -128553) >> 16)
                + (((c + b) * -58980) >> 16);
            v = n
                + ((d * 134553) >> 16)
                + (((d + b) * -25570) >> 16)
                + (((d + a) * -167963) >> 16);
            t = n
                + ((b * 98390) >> 16)
                + (((d + b) * -25570) >> 16)
                + (((c + b) * -58980) >> 16);
            s = n
                + ((a * 201373) >> 16)
                + (((c + a) * -128553) >> 16)
                + (((d + a) * -167963) >> 16);

            lp1[0x00] = w + t;
            lp1[0x38] = w - t;
            lp1[0x08] = y + s;
            lp1[0x30] = y - s;
            lp1[0x10] = z + v;
            lp1[0x28] = z - v;
            lp1[0x18] = x + u;
            lp1[0x20] = x - u;
        }

        lp1++;
        lp2++;
    }

    lp1 = dct_table.data();

    for (const auto i : algo::range(8))
    {
        a = lp1[0];
        c = lp1[2];
        b = lp1[4];
        d = lp1[6];
        x = (((c + d) * 35467) >> 16);
        c = ((c * 50159) >> 16) + x;
        d = ((d * -121094) >> 16) + x;
        w = a + b + c;
        x = a + b - c;
        y = a - b + d;
        z = a - b - d;

        d = lp1[5];
        b = lp1[1];
        c = lp1[7];
        a = lp1[3];
        n = (((a + b + c + d) * 77062) >> 16);

        s = n + ((a * 201373) >> 16)
              + (((a + c) * -128553) >> 16)
              + (((a + d) * -167963) >> 16);

        t = n + ((b * 98390) >> 16)
              + (((b + d) * -25570) >> 16)
              + (((b + c) * -58980) >> 16);

        u = n + ((c * 19571) >> 16)
              + (((b + c) * -58980) >> 16)
              + (((a + c) * -128553) >> 16);

        v = n + ((d * 134553) >> 16)
              + (((b + d) * -25570) >> 16)
              + (((a + d) * -167963) >> 16);

        lp1[0] = (w + t) >> 3;
        lp1[7] = (w - t) >> 3;
        lp1[1] = (y + s) >> 3;
        lp1[6] = (y - s) >> 3;
        lp1[2] = (z + v) >> 3;
        lp1[5] = (z - v) >> 3;
        lp1[3] = (x + u) >> 3;
        lp1[4] = (x - u) >> 3;

        lp1 += 8;
    }
}

static void ycc2rgb(u8 *dc, u8 *ac, short *iy, short *cbcr, const size_t stride)
{
    static bool initialized = false;
    static std::array<u8, 0x300> lookup_table;

    if (!initialized)
    {
        for (const auto n : algo::range(0x100))
            lookup_table[n] = 0;

        for (const auto n : algo::range(0x100))
            lookup_table[n + 0x100] = n;

        for (const auto n : algo::range(0x100))
            lookup_table[n + 0x200] = 0xFF;

        initialized = true;
    }

    for (const auto y : algo::range(4))
    {
        for (const auto x : algo::range(4))
        {
            const auto c = cbcr[0];
            const auto d = cbcr[-64];
            const auto r = ((c * 0x166F0) >> 16);
            const auto g = ((d * 0x5810) >> 16) + ((c * 0xB6C0) >> 16);
            const auto b = ((d * 0x1C590) >> 16);
            const auto cw = iy[1] + 0x180;
            const auto cx = iy[0] + 0x180;
            const auto cy = iy[8] + 0x180;
            const auto cz = iy[9] + 0x180;

            dc[0]          = lookup_table.at(cx + b);
            ac[4 - stride] = lookup_table.at(cw + b);
            ac[0]          = lookup_table.at(cy + b);
            ac[4]          = lookup_table.at(cz + b);
            ac[1 - stride] = lookup_table.at(cx - g);
            ac[5 - stride] = lookup_table.at(cw - g);
            ac[1]          = lookup_table.at(cy - g);
            ac[5]          = lookup_table.at(cz - g);
            ac[2 - stride] = lookup_table.at(cx + r);
            ac[6 - stride] = lookup_table.at(cw + r);
            ac[2]          = lookup_table.at(cy + r);
            ac[6]          = lookup_table.at(cz + r);
            iy += 2;
            dc += 8;
            ac += 8;
            cbcr++;
        }

        cbcr -= 4;
        dc -= 8 * 4;
        ac -= 8 * 4;

        dc += stride * 2;
        ac += stride * 2;

        iy += 8;
        cbcr += 8;
    }
}

static bstr decode_blocks(
    const BasicInfo &info,
    const bstr &tree_input,
    io::BaseBitStream &bit_stream_1,
    io::BaseBitStream &bit_stream_2,
    std::array<u32, 0x80> &freq_dc,
    std::array<u32, 0x80> &freq_ac,
    const std::array<s16, 64> &quant_y,
    const std::array<s16, 64> &quant_c)
{
    const auto tree_dc = make_tree(tree_input, freq_dc);
    const auto tree_ac = make_tree(tree_input, freq_ac);

    std::vector<u32> tmp(info.x_block_count * info.y_block_count * 3 * 2);

    for (const auto i : algo::range(tmp.size()))
    {
        const auto bit_count = read_from_tree(tree_dc, bit_stream_1);
        u32 x = bit_stream_1.read(bit_count);
        if (x < (1u << (bit_count - 1)))
            x = x - (1 << bit_count) + 1;

        tmp[i] = x;
        if (i)
            tmp[i] += tmp[i - 1];
    }

    bstr block_output(info.blocks_width * info.blocks_height * 4);
    static const u8 original_order[64] =
    {
        1,  8,  16, 9,  2,  3,  10, 17,
        24, 32, 25, 18, 11, 4,  5,  12,
        19, 26, 33, 40, 48, 41, 34, 27,
        20, 13, 6,  7,  14, 21, 28, 35,
        42, 49, 56, 57, 50, 43, 36, 29,
        22, 15, 23, 30, 37, 44, 51, 58,
        59, 52, 45, 38, 31, 39, 46, 53,
        60, 61, 54, 47, 55, 62, 63, 0
    };

    for (const auto y : algo::range(info.y_block_count))
    {
        auto target_base = &block_output[(info.blocks_width * 64) * y];
        u8 *target1 = target_base + 32;
        u8 *target2 = target_base + info.block_stride * 9;

        for (const auto x : algo::range(info.x_block_count))
        {
            std::array<s16, 64> dct_table[6] = {0, 0, 0, 0, 0, 0};

            for (const auto n : algo::range(6))
            {
                dct_table[n][0] = tmp.at((y * info.x_block_count + x) * 6 + n);

                for (int i = 0; i < 63;)
                {
                    const auto bit_count
                        = read_from_tree(tree_ac, bit_stream_2);

                    if (bit_count == 15)
                        break;

                    if (!bit_count)
                    {
                        auto tree_input_pos = 0;
                        while (bit_stream_2.read(1))
                            tree_input_pos++;
                        i += tree_input.at(tree_input_pos);
                    }
                    else
                    {
                        u32 x = bit_stream_2.read(bit_count);
                        if (x < (1u << (bit_count - 1)))
                            x = x - (1 << bit_count) + 1;
                        dct_table[n][original_order[i]] = x;
                        i++;
                    }
                }
            }

            dct(dct_table[0], quant_y);
            dct(dct_table[1], quant_y);
            dct(dct_table[2], quant_y);
            dct(dct_table[3], quant_y);
            dct(dct_table[4], quant_c);
            dct(dct_table[5], quant_c);

            u8 *dc, *ac;

            dc = target1 - 32;
            ac = target1 - 32 + info.block_stride;
            ycc2rgb(
                dc, ac, &dct_table[0][0], &dct_table[5][0], info.block_stride);

            dc = target1;
            ac = target2 + 32 - info.block_stride * 8;
            ycc2rgb(
                dc, ac, &dct_table[1][0], &dct_table[5][4], info.block_stride);

            dc = target1 + ((info.block_stride) << 3) - 32;
            ac = target2;
            ycc2rgb(
                dc, ac, &dct_table[2][0], &dct_table[5][32], info.block_stride);

            dc = target2 + 32 - info.block_stride;
            ac = target2 + 32;
            ycc2rgb(
                dc, ac, &dct_table[3][0], &dct_table[5][36], info.block_stride);

            target1 += 64;
            target2 += 64;
        }
    }
    return block_output;
}

static BasicInfo read_basic_info(io::BaseByteStream &input_stream)
{
    input_stream.seek(4);

    BasicInfo info;
    info.data_offset = input_stream.read_le<u32>();
    info.flags = input_stream.read_le<u32>();
    input_stream.skip(4);
    info.width = input_stream.read_le<u16>();
    info.height = input_stream.read_le<u16>();
    info.depth = input_stream.read_le<u16>();
    input_stream.skip(6);
    info.bit_pool_1_size = input_stream.read_le<u32>();
    info.bit_pool_2_size = input_stream.read_le<u32>();

    const auto type = (info.flags >> 28) & 3;
    if (type == 0)
    {
        info.x_block_size = 8;
        info.y_block_size = 8;
    }
    else if (type == 1)
    {
        info.x_block_size = 16;
        info.y_block_size = 16;
    }
    else if (type == 2)
    {
        info.x_block_size = 32;
        info.y_block_size = 16;
    }
    else
    {
        throw err::NotSupportedError("Unknown JBP1 type");
    }

    info.blocks_width
        = (info.width + info.x_block_size - 1) & ~(info.x_block_size - 1);
    info.blocks_height
        = (info.height + info.y_block_size - 1) & ~(info.y_block_size - 1);
    info.block_stride = info.blocks_width * 4;
    info.x_block_count = info.blocks_width >> 4;
    info.y_block_count = info.blocks_height >> 4;

    return info;
}

bstr dec::purple_software::jbp1_decompress(const bstr &input)
{
    io::MemoryByteStream input_stream(input);
    const auto info = read_basic_info(input_stream);

    input_stream.seek(info.data_offset);

    std::array<u32, 0x80> freq_dc;
    for (const auto i : algo::range(16))
        freq_dc[i] = input_stream.read_le<u32>();

    std::array<u32, 0x80> freq_ac;
    for (const auto i : algo::range(16))
        freq_ac[i] = input_stream.read_le<u32>();

    bstr tree_input = input_stream.read(16);
    for (auto &c : tree_input)
        c++;

    std::array<s16, 64> quant_y;
    std::array<s16, 64> quant_c;
    if (info.flags & 0x08000000)
    {
        for (const auto i : algo::range(64))
            quant_y[i] = input_stream.read<u8>();

        for (const auto i : algo::range(64))
            quant_c[i] =  input_stream.read<u8>();
    }

    CustomBitStream bit_stream_1(input_stream.read(info.bit_pool_1_size));
    CustomBitStream bit_stream_2(input_stream.read(info.bit_pool_2_size));
    const auto block_output = decode_blocks(
        info,
        tree_input,
        bit_stream_1,
        bit_stream_2,
        freq_dc,
        freq_ac,
        quant_y,
        quant_c);

    const auto channel_count = info.depth >> 3;
    bstr pixel_output(info.width * info.height * channel_count);
    auto pixel_output_ptr = &pixel_output[0];
    for (const auto y : algo::range(info.height))
    {
        auto block_output_ptr = &block_output[info.block_stride * y];
        for (const auto x : algo::range(info.width))
        {
            for (const auto channel : algo::range(channel_count))
            {
                *pixel_output_ptr++ = block_output_ptr[channel];
            }
            block_output_ptr += 4;
        }
    }
    return pixel_output;
}
