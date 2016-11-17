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

#include "dec/kirikiri/tlg/tlg6_decoder.h"
#include "algo/range.h"
#include "dec/kirikiri/tlg/lzss_decompressor.h"
#include "err.h"

using namespace au;
using namespace au::dec::kirikiri::tlg;

static const int w_block_size = 8;
static const int h_block_size = 8;
static const int golomb_n_count = 4;
static const int leading_zero_table_bits = 12;
static const int leading_zero_table_size = (1 << leading_zero_table_bits);

static u8 leading_zero_table[leading_zero_table_size];
static u8 golomb_bit_size_table[golomb_n_count * 2 * 128][golomb_n_count];

namespace
{
    struct Header final
    {
        u8 channel_count;
        u8 data_flags;
        u8 color_type;
        u8 external_golomb_table;
        size_t image_width;
        size_t image_height;
        size_t max_bit_size;
        size_t x_block_count;
        size_t y_block_count;
    };

    struct FilterTypes final
    {
        FilterTypes(io::BaseByteStream &input_stream);
        void decompress(const Header &header);

        bstr data;
    };
}

FilterTypes::FilterTypes(io::BaseByteStream &input_stream)
{
    const auto size = input_stream.read_le<u32>();
    data = input_stream.read(size);
}

void FilterTypes::decompress(const Header &header)
{
    LzssDecompressor decompressor;
    u8 dictionary[4096];
    u8 *ptr = dictionary;
    for (const auto i : algo::range(32))
    {
        for (const auto j : algo::range(16))
        {
            for (const auto k : algo::range(4))
                *ptr++ = i;

            for (const auto k : algo::range(4))
                *ptr++ = j;
        }
    }
    decompressor.init_dictionary(dictionary);

    size_t output_size = header.x_block_count * header.y_block_count;
    data = decompressor.decompress(data, output_size);
}

static inline u32 to_bgra(res::Pixel p)
{
    return p.b | (p.g << 8) | (p.r << 16) | (p.a << 24);
}

static void transformer0(res::Pixel &)
{
}

static void transformer1(res::Pixel &p)
{
    p.r += p.g;
    p.b += p.g;
}

static void transformer2(res::Pixel &p)
{
    p.g += p.b;
    p.r += p.g;
}

static void transformer3(res::Pixel &p)
{
    p.g += p.r;
    p.b += p.g;
}

static void transformer4(res::Pixel &p)
{
    p.b += p.r;
    p.g += p.b;
    p.r += p.g;
}

static void transformer5(res::Pixel &p)
{
    p.b += p.r;
    p.g += p.b;
}

static void transformer6(res::Pixel &p)
{
    p.b += p.g;
}

static void transformer7(res::Pixel &p)
{
    p.g += p.b;
}

static void transformer8(res::Pixel &p)
{
    p.r += p.g;
}

static void transformer9(res::Pixel &p)
{
    p.r += p.b;
    p.g += p.r;
    p.b += p.g;
}

static void transformerA(res::Pixel &p)
{
    p.b += p.r;
    p.g += p.r;
}

static void transformerB(res::Pixel &p)
{
    p.r += p.b;
    p.g += p.b;
}

static void transformerC(res::Pixel &p)
{
    p.r += p.b;
    p.g += p.r;
}

static void transformerD(res::Pixel &p)
{
    p.b += p.g;
    p.r += p.b;
    p.g += p.r;
}

static void transformerE(res::Pixel &p)
{
    p.g += p.r;
    p.b += p.g;
    p.r += p.b;
}

static void transformerF(res::Pixel &p)
{
    p.g += (p.b << 1);
    p.r += (p.b << 1);
}

static void (*transformers[16])(res::Pixel &p) =
{
    &transformer0, &transformer1, &transformer2, &transformer3,
    &transformer4, &transformer5, &transformer6, &transformer7,
    &transformer8, &transformer9, &transformerA, &transformerB,
    &transformerC, &transformerD, &transformerE, &transformerF,
};

static inline u32 make_gt_mask(u32 a, u32 b)
{
    u32 tmp2 = ~b;
    u32 tmp = ((a & tmp2) + (((a ^ tmp2) >> 1) & 0x7F7F7F7F)) & 0x80808080;
    return ((tmp >> 7) + 0x7F7F7F7F) ^ 0x7F7F7F7F;
}

static inline u32 packed_bytes_add(u32 a, u32 b)
{
    return a + b - ((((a & b) << 1) + ((a ^ b) & 0xFEFEFEFE)) & 0x01010100);
}

static inline u32 med(u32 a, u32 b, u32 c, u32 v)
{
    u32 aa_gt_bb = make_gt_mask(a, b);
    u32 a_xor_b_and_aa_gt_bb = ((a ^ b) & aa_gt_bb);
    u32 aa = a_xor_b_and_aa_gt_bb ^ a;
    u32 bb = a_xor_b_and_aa_gt_bb ^ b;
    u32 n = make_gt_mask(c, bb);
    u32 nn = make_gt_mask(aa, c);
    u32 m = ~(n | nn);
    return packed_bytes_add(
        (n & aa) | (nn & bb) | ((bb & m) - (c & m) + (aa & m)), v);
}

static inline u32 avg(u32 a, u32 b, u32, u32 v)
{
    return packed_bytes_add((a & b)
        + (((a ^ b) & 0xFEFEFEFE) >> 1)
        + ((a ^ b) & 0x01010101), v);
}

static void init_table()
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;

    short golomb_compression_table[golomb_n_count][9] =
    {
        {3, 7, 15, 27, 63, 108, 223, 448, 130},
        {3, 5, 13, 24, 51, 95, 192, 384, 257},
        {2, 5, 12, 21, 39, 86, 155, 320, 384},
        {2, 3, 9, 18, 33, 61, 129, 258, 511},
    };

    for (const auto i : algo::range(leading_zero_table_size))
    {
        int cnt = 0;
        int j = 1;

        while (j != leading_zero_table_size && !(i & j))
        {
            j <<= 1;
            cnt++;
        }

        cnt++;

        if (j == leading_zero_table_size)
            cnt = 0;

        leading_zero_table[i] = cnt;
    }

    for (const auto n : algo::range(golomb_n_count))
    {
        int a = 0;
        for (const auto i : algo::range(9))
        {
            for (const auto j : algo::range(golomb_compression_table[n][i]))
                golomb_bit_size_table[a++][n] = i;
        }
    }
}

static void decode_golomb_values(u8 *pixel_buf, int pixel_count, u8 *bit_pool)
{
    int n = golomb_n_count - 1;
    int a = 0;

    int bit_pos = 1;
    u8 zero = (*bit_pool & 1) ? 0 : 1;
    u8 *limit = pixel_buf + pixel_count * 4;

    while (pixel_buf < limit)
    {
        int count;
        {
            u32 t = *reinterpret_cast<u32*>(bit_pool) >> bit_pos;
            int b = leading_zero_table[t & (leading_zero_table_size - 1)];
            int bit_count = b;
            while (!b)
            {
                bit_count += leading_zero_table_bits;
                bit_pos += leading_zero_table_bits;
                bit_pool += bit_pos >> 3;
                bit_pos &= 7;
                t = *reinterpret_cast<u32*>(bit_pool) >> bit_pos;
                b = leading_zero_table[t & (leading_zero_table_size - 1)];
                bit_count += b;
            }

            bit_pos += b;
            bit_pool += bit_pos >> 3;
            bit_pos &= 7;

            bit_count--;
            count = 1 << bit_count;
            t = *reinterpret_cast<u32*>(bit_pool);
            count += ((t >> bit_pos) & (count - 1));

            bit_pos += bit_count;
            bit_pool += bit_pos >> 3;
            bit_pos &= 7;
        }

        if (zero)
        {
            do
            {
                *pixel_buf = 0;
                pixel_buf += 4;
            }
            while (--count && pixel_buf < limit);
        }
        else
        {
            do
            {
                int bit_count;
                int b;

                u32 t = *reinterpret_cast<u32*>(bit_pool);
                t >>= bit_pos;
                if (t)
                {
                    b = leading_zero_table[
                        t & (leading_zero_table_size - 1)];
                    bit_count = b;
                    while (!b)
                    {
                        bit_count += leading_zero_table_bits;
                        bit_pos += leading_zero_table_bits;
                        bit_pool += bit_pos >> 3;
                        bit_pos &= 7;
                        t = *reinterpret_cast<u32*>(bit_pool);
                        t >>= bit_pos;
                        b = leading_zero_table[
                            t & (leading_zero_table_size - 1)];
                        bit_count += b;
                    }
                    bit_count--;
                }
                else
                {
                    bit_pool += 5;
                    bit_count = bit_pool[-1];
                    bit_pos = 0;
                    t = *reinterpret_cast<u32*>(bit_pool);
                    b = 0;
                }

                if (a >= golomb_n_count * 2 * 128) a = 0;
                if (n >= golomb_n_count) n = 0;

                int k = golomb_bit_size_table[a][n];
                int v = (bit_count << k) + ((t >> b) & ((1 << k) - 1));
                int sign = (v & 1) - 1;
                v >>= 1;
                a += v;

                *reinterpret_cast<u8*>(pixel_buf) = ((v ^ sign) + sign + 1);
                pixel_buf += 4;

                bit_pos += b;
                bit_pos += k;
                bit_pool += bit_pos >> 3;
                bit_pos &= 7;

                if (--n < 0)
                {
                    a >>= 1;
                    n = golomb_n_count - 1;
                }
            }
            while (--count && pixel_buf < limit);
        }

        zero ^= 1;
    }
}

static void decode_line(
    res::Pixel *prev_line,
    res::Pixel *current_line,
    int start_block,
    int block_limit,
    u8 *filter_types,
    int skip_block_bytes,
    u32 *in,
    int odd_skip,
    int dir,
    const Header &header)
{
    res::Pixel left, top_left;
    int step;

    if (start_block)
    {
        prev_line += start_block * w_block_size;
        current_line += start_block * w_block_size;
        left = current_line[-1];
        top_left = prev_line[-1];
    }
    else
    {
        left.r = left.g = left.b = 0;
        top_left.r = top_left.g = top_left.b = 0;
        left.a = top_left.a = header.channel_count == 3 ? 0xFF : 0;
    }

    in += skip_block_bytes * start_block;
    step = (dir & 1) ? 1 : -1;

    for (const auto i : algo::range(start_block, block_limit))
    {
        int w = header.image_width - i * w_block_size;
        if (w > w_block_size)
            w = w_block_size;

        int ww = w;
        if (step == -1)
            in += ww - 1;

        if (i & 1)
            in += odd_skip * ww;

        auto filter = filter_types[i] & 1 ? &avg : &med;
        auto transformer = transformers[filter_types[i] >> 1];

        do
        {
            res::Pixel inn;
            inn.a = *in >> 24;
            inn.r = *in >> 16;
            inn.g = *in >> 8;
            inn.b = *in;
            transformer(inn);

            res::Pixel top = *prev_line;
            auto result = filter(
                to_bgra(left),
                to_bgra(top),
                to_bgra(top_left),
                to_bgra(inn));
            left.a = result >> 24;
            left.r = result >> 16;
            left.g = result >> 8;
            left.b = result;
            if (header.channel_count == 3)
                left.a = 0xFF;

            top_left = top;
            *current_line++ = left;

            prev_line++;
            in += step;
        }
        while (--w);

        in += skip_block_bytes + (step == 1 ? - ww : 1);
        if (i & 1)
            in -= odd_skip * ww;
    }
}

static void read_image(
    io::BaseByteStream &input_stream, res::Image &image, const Header &header)
{
    FilterTypes filter_types(input_stream);
    filter_types.decompress(header);

    bstr pixel_buf(4 * header.image_width * h_block_size);
    auto zero_line = std::make_unique<res::Pixel[]>(header.image_width);
    res::Pixel *prev_line = zero_line.get();

    u32 main_count = header.image_width / w_block_size;
    for (const auto y : algo::range(0, header.image_height, h_block_size))
    {
        u32 ylim = y + h_block_size;
        if (ylim >= header.image_height)
            ylim = header.image_height;

        int pixel_count = (ylim - y) * header.image_width;
        for (const auto c : algo::range(header.channel_count))
        {
            u32 bit_size = input_stream.read_le<u32>();

            int method = (bit_size >> 30) & 3;
            bit_size &= 0x3FFFFFFF;

            int byte_size = (bit_size + 7) / 8;
            bstr bit_pool = input_stream.read(byte_size);

            // Although decode_golomb_values accesses only valid bits, it uses
            // reinterpret_cast<u32*>() that might access bits out of bounds.
            // This is to make sure those calls don't cause access violation.
            bit_pool.resize(byte_size + 4);

            if (method != 0)
                throw err::NotSupportedError("Unsupported encoding method");

            decode_golomb_values(
                pixel_buf.get<u8>() + c, pixel_count, bit_pool.get<u8>());
        }

        u8 *ft = filter_types.data.get<u8>()
            + (y / h_block_size) * header.x_block_count;
        int skip_bytes = (ylim - y) * w_block_size;

        for (const auto yy : algo::range(y, ylim))
        {
            auto *current_line = &image.at(0, yy);

            int dir = (yy & 1) ^ 1;
            int odd_skip = ((ylim - yy -1) - (yy - y));

            if (main_count)
            {
                int start = ((header.image_width < w_block_size)
                    ? header.image_width
                    : w_block_size) * (yy - y);

                decode_line(
                    prev_line,
                    current_line,
                    0,
                    main_count,
                    ft,
                    skip_bytes,
                    pixel_buf.get<u32>() + start,
                    odd_skip,
                    dir,
                    header);
            }

            if (main_count != header.x_block_count)
            {
                int ww = header.image_width - main_count * w_block_size;
                if (ww > w_block_size)
                    ww = w_block_size;

                int start = ww * (yy - y);
                decode_line(
                    prev_line,
                    current_line,
                    main_count,
                    header.x_block_count,
                    ft,
                    skip_bytes,
                    pixel_buf.get<u32>() + start,
                    odd_skip,
                    dir,
                    header);
            }

            prev_line = current_line;
        }
    }
}

res::Image Tlg6Decoder::decode(io::File &file)
{
    init_table();

    Header header;
    header.channel_count = file.stream.read<u8>();
    header.data_flags = file.stream.read<u8>();
    header.color_type = file.stream.read<u8>();
    header.external_golomb_table = file.stream.read<u8>();
    header.image_width = file.stream.read_le<u32>();
    header.image_height = file.stream.read_le<u32>();
    header.max_bit_size = file.stream.read_le<u32>();

    header.x_block_count = ((header.image_width - 1) / w_block_size) + 1;
    header.y_block_count = ((header.image_height - 1) / h_block_size) + 1;
    if (header.channel_count != 3 && header.channel_count != 4)
        throw err::UnsupportedChannelCountError(header.channel_count);

    res::Image image(header.image_width, header.image_height);
    read_image(file.stream, image, header);
    return image;
}
