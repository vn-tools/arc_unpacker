#include <stdexcept>
#include "fmt/kirikiri/tlg/lzss_decompressor.h"
#include "fmt/kirikiri/tlg/tlg6_decoder.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::kirikiri::tlg;

static const int W_BLOCK_SIZE = 8;
static const int H_BLOCK_SIZE = 8;
static const int GOLOMB_N_COUNT = 4;
static const int LEADING_ZERO_TABLE_BITS = 12;
static const int LEADING_ZERO_TABLE_SIZE = (1 << LEADING_ZERO_TABLE_BITS);

static u8 leading_zero_table[LEADING_ZERO_TABLE_SIZE];
static u8 golomb_bit_length_table[GOLOMB_N_COUNT * 2 * 128][GOLOMB_N_COUNT];

namespace
{
    struct Header
    {
        u8 channel_count;
        u8 data_flags;
        u8 color_type;
        u8 external_golomb_table;
        size_t image_width;
        size_t image_height;
        size_t max_bit_length;

        size_t x_block_count;
        size_t y_block_count;
    };

    struct FilterTypes
    {
        u32 data_size;
        std::unique_ptr<u8[]> data;
        FilterTypes(io::IO &io);
        void decompress(Header &header);
    };
}

FilterTypes::FilterTypes(io::IO &io) : data(nullptr)
{
    data_size = io.read_u32_le();
    data.reset(new u8[data_size]);
    io.read(data.get(), data_size);
}

void FilterTypes::decompress(Header &header)
{
    size_t output_size = header.x_block_count * header.y_block_count;
    std::unique_ptr<u8[]> output(new u8[output_size]);

    LzssDecompressor decompressor;
    u8 dictionary[4096];
    u8 *ptr = dictionary;
    for (auto i : util::range(32))
    {
        for (auto j : util::range(16))
        {
            for (auto k : util::range(4))
                *ptr++ = i;

            for (auto k : util::range(4))
                *ptr++ = j;
        }
    }
    decompressor.init_dictionary(dictionary);

    decompressor.decompress(
        data.get(),
        data_size,
        output.get(),
        output_size);

    data = std::move(output);
    data_size = output_size;
}

static void transformer1(u8 &, u8 &, u8 &)
{
}

static void transformer2(u8 &r, u8 &g, u8 &b)
{
    r += g;
    b += g;
}

static void transformer3(u8 &r, u8 &g, u8 &b)
{
    g += b;
    r += g;
}

static void transformer4(u8 &r, u8 &g, u8 &b)
{
    g += r;
    b += g;
}

static void transformer5(u8 &r, u8 &g, u8 &b)
{
    b += r;
    g += b;
    r += g;
}

static void transformer6(u8 &r, u8 &g, u8 &b)
{
    b += r;
    g += b;
}

static void transformer7(u8 &, u8 &g, u8 &b)
{
    b += g;
}

static void transformer8(u8 &, u8 &g, u8 &b)
{
    g += b;
}

static void transformer9(u8 &r, u8 &g, u8 &)
{
    r += g;
}

static void transformer10(u8 &r, u8 &g, u8 &b)
{
    r += b;
    g += r;
    b += g;
}

static void transformer11(u8 &r, u8 &g, u8 &b)
{
    b += r;
    g += r;
}

static void transformer12(u8 &r, u8 &g, u8 &b)
{
    r += b;
    g += b;
}

static void transformer13(u8 &r, u8 &g, u8 &b)
{
    r += b;
    g += r;
}

static void transformer14(u8 &r, u8 &g, u8 &b)
{
    b += g;
    r += b;
    g += r;
}

static void transformer15(u8 &r, u8 &g, u8 &b)
{
    g += r;
    b += g;
    r += b;
}

static void transformer16(u8 &r, u8 &g, u8 &b)
{
    g += (b << 1);
    r += (b << 1);
}

static void (*transformers[16])(u8 &, u8 &, u8 &) =
{
    &transformer1, &transformer2, &transformer3, &transformer4,
    &transformer5, &transformer6, &transformer7, &transformer8,
    &transformer9, &transformer10, &transformer11, &transformer12,
    &transformer13, &transformer14, &transformer15, &transformer16,
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

    short golomb_compression_table[GOLOMB_N_COUNT][9] =
    {
        {3, 7, 15, 27, 63, 108, 223, 448, 130, },
        {3, 5, 13, 24, 51, 95, 192, 384, 257, },
        {2, 5, 12, 21, 39, 86, 155, 320, 384, },
        {2, 3, 9, 18, 33, 61, 129, 258, 511, },
    };

    for (auto i : util::range(LEADING_ZERO_TABLE_SIZE))
    {
        int cnt = 0;
        int j = 1;

        while (j != LEADING_ZERO_TABLE_SIZE && !(i & j))
        {
            j <<= 1;
            cnt++;
        }

        cnt++;

        if (j == LEADING_ZERO_TABLE_SIZE)
            cnt = 0;

        leading_zero_table[i] = cnt;
    }

    for (auto n : util::range(GOLOMB_N_COUNT))
    {
        int a = 0;
        for (auto i : util::range(9))
        {
            for (auto j : util::range(golomb_compression_table[n][i]))
                golomb_bit_length_table[a++][n] = i;
        }
    }
}

static void decode_golomb_values(u8 *pixel_buf, int pixel_count, u8 *bit_pool)
{
    int n = GOLOMB_N_COUNT - 1;
    int a = 0;

    int bit_pos = 1;
    u8 zero = (*bit_pool & 1) ? 0 : 1;
    u8 *limit = pixel_buf + pixel_count * 4;

    while (pixel_buf < limit)
    {
        int count;
        {
            u32 t = *reinterpret_cast<u32*>(bit_pool) >> bit_pos;
            int b = leading_zero_table[t & (LEADING_ZERO_TABLE_SIZE - 1)];
            int bit_count = b;
            while (!b)
            {
                bit_count += LEADING_ZERO_TABLE_BITS;
                bit_pos += LEADING_ZERO_TABLE_BITS;
                bit_pool += bit_pos >> 3;
                bit_pos &= 7;
                t = *reinterpret_cast<u32*>(bit_pool) >> bit_pos;
                b = leading_zero_table[t & (LEADING_ZERO_TABLE_SIZE - 1)];
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
                        t & (LEADING_ZERO_TABLE_SIZE - 1)];
                    bit_count = b;
                    while (!b)
                    {
                        bit_count += LEADING_ZERO_TABLE_BITS;
                        bit_pos += LEADING_ZERO_TABLE_BITS;
                        bit_pool += bit_pos >> 3;
                        bit_pos &= 7;
                        t = *reinterpret_cast<u32*>(bit_pool);
                        t >>= bit_pos;
                        b = leading_zero_table[
                            t & (LEADING_ZERO_TABLE_SIZE - 1)];
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

                if (a >= GOLOMB_N_COUNT * 2 * 128) a = 0;
                if (n >= GOLOMB_N_COUNT) n = 0;

                int k = golomb_bit_length_table[a][n];
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
                    n = GOLOMB_N_COUNT - 1;
                }
            }
            while (--count);
        }

        zero ^= 1;
    }
}

static void decode_line(
    u32 *prev_line,
    u32 *current_line,
    int start_block,
    int block_limit,
    u8 *filter_types,
    int skip_block_bytes,
    u32 *in,
    int odd_skip,
    int dir,
    Header &header)
{
    u32 p, up;
    int step;

    if (start_block)
    {
        prev_line += start_block * W_BLOCK_SIZE;
        current_line += start_block * W_BLOCK_SIZE;
        p = current_line[-1];
        up = prev_line[-1];
    }
    else
    {
        p = up = header.channel_count == 3 ? 0xFF000000 : 0;
    }

    in += skip_block_bytes * start_block;
    step = (dir & 1) ? 1 : -1;

    for (auto i : util::range(start_block, block_limit))
    {
        int w = header.image_width - i * W_BLOCK_SIZE;
        if (w > W_BLOCK_SIZE)
            w = W_BLOCK_SIZE;

        int ww = w;
        if (step == -1)
            in += ww - 1;

        if (i & 1)
            in += odd_skip * ww;

        auto filter = filter_types[i] & 1 ? &avg : &med;
        auto transformer = transformers[filter_types[i] >> 1];

        do
        {
            u8 a = (*in >> 24) & 0xFF;
            u8 r = (*in >> 16) & 0xFF;
            u8 g = (*in >> 8) & 0xFF;
            u8 b = (*in) & 0xFF;

            transformer(r, g, b);

            u32 u = *prev_line;
            p = filter(
                p,
                u,
                up,
                (0xFF0000 & (b << 16))
                + (0xFF00 & (g << 8))
                + (0xFF & r) + (a << 24));

            if (header.channel_count == 3)
                p |= 0xFF000000;

            up = u;
            *current_line++ = p;

            prev_line++;
            in += step;
        }
        while (--w);

        in += skip_block_bytes + (step == 1 ? - ww : 1);
        if (i & 1)
            in -= odd_skip * ww;
    }
}

static void read_pixels(io::IO &io, u8 *output, Header &header)
{
    FilterTypes filter_types(io);
    filter_types.decompress(header);

    std::unique_ptr<u32[]> pixel_buf(
        new u32[4 * header.image_width * H_BLOCK_SIZE]);
    std::unique_ptr<u32[]> zero_line(new u32[header.image_width]());
    u32 *prev_line = zero_line.get();

    u32 main_count = header.image_width / W_BLOCK_SIZE;
    for (auto y : util::range(0, header.image_height, H_BLOCK_SIZE))
    {
        u32 ylim = y + H_BLOCK_SIZE;
        if (ylim >= header.image_height)
            ylim = header.image_height;

        int pixel_count = (ylim - y) * header.image_width;
        for (auto c : util::range(header.channel_count))
        {
            u32 bit_length = io.read_u32_le();

            int method = (bit_length >> 30) & 3;
            bit_length &= 0x3FFFFFFF;

            int byte_length = bit_length / 8;
            if (bit_length % 8)
                byte_length++;

            std::unique_ptr<u8[]> bit_pool(new u8[byte_length]);
            io.read(bit_pool.get(), byte_length);

            if (method != 0)
                throw std::runtime_error("Unsupported encoding method");

            decode_golomb_values(
                reinterpret_cast<u8*>(pixel_buf.get()) + c,
                pixel_count,
                bit_pool.get());
        }

        u8 *ft = filter_types.data.get()
            + (y / H_BLOCK_SIZE) * header.x_block_count;
        int skip_bytes = (ylim - y) * W_BLOCK_SIZE;

        for (auto yy : util::range(y, ylim))
        {
            u32 *current_line
                = &reinterpret_cast<u32*>(output)
                    [yy * header.image_width];

            int dir = (yy & 1) ^ 1;
            int odd_skip = ((ylim - yy -1) - (yy - y));

            if (main_count)
            {
                int start = ((header.image_width < W_BLOCK_SIZE)
                    ? header.image_width
                    : W_BLOCK_SIZE) * (yy - y);

                decode_line(
                    prev_line,
                    current_line,
                    0,
                    main_count,
                    ft,
                    skip_bytes,
                    pixel_buf.get() + start,
                    odd_skip,
                    dir,
                    header);
            }

            if (main_count != header.x_block_count)
            {
                int ww = header.image_width - main_count * W_BLOCK_SIZE;
                if (ww > W_BLOCK_SIZE)
                    ww = W_BLOCK_SIZE;

                int start = ww * (yy - y);
                decode_line(
                    prev_line,
                    current_line,
                    main_count,
                    header.x_block_count,
                    ft,
                    skip_bytes,
                    pixel_buf.get() + start,
                    odd_skip,
                    dir,
                    header);
            }

            prev_line = current_line;
        }
    }
}

std::unique_ptr<File> Tlg6Decoder::decode(File &file)
{
    init_table();

    Header header;
    header.channel_count = file.io.read_u8();
    header.data_flags = file.io.read_u8();
    header.color_type = file.io.read_u8();
    header.external_golomb_table = file.io.read_u8();
    header.image_width = file.io.read_u32_le();
    header.image_height = file.io.read_u32_le();
    header.max_bit_length = file.io.read_u32_le();

    header.x_block_count = ((header.image_width - 1) / W_BLOCK_SIZE) + 1;
    header.y_block_count = ((header.image_height - 1) / H_BLOCK_SIZE) + 1;
    if (header.channel_count != 3 && header.channel_count != 4)
        throw std::runtime_error("Unsupported channel count");

    size_t pixels_size = header.image_width * header.image_height * 4;
    std::unique_ptr<u8[]> pixels(new u8[pixels_size]());

    read_pixels(file.io, pixels.get(), header);

    std::unique_ptr<util::Image> image = util::Image::from_pixels(
        header.image_width,
        header.image_height,
        std::string(reinterpret_cast<char*>(pixels.get()), pixels_size),
        util::PixelFormat::RGBA);
    return image->create_file(file.name);
}
