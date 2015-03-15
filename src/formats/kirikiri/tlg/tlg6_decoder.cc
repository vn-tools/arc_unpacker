#include <stdexcept>
#include "formats/image.h"
#include "formats/kirikiri/tlg/lzss_decompressor.h"
#include "formats/kirikiri/tlg/tlg6_decoder.h"
using namespace Formats::Kirikiri::Tlg;

namespace
{
    typedef unsigned char uchar;
    typedef unsigned long ulong;

    const int W_BLOCK_SIZE = 8;
    const int H_BLOCK_SIZE = 8;
    const int GOLOMB_N_COUNT = 4;
    const int LEADING_ZERO_TABLE_BITS = 12;
    const int LEADING_ZERO_TABLE_SIZE = (1 << LEADING_ZERO_TABLE_BITS);

    uchar leading_zero_table[LEADING_ZERO_TABLE_SIZE];
    uchar golomb_bit_length_table[GOLOMB_N_COUNT * 2 * 128][GOLOMB_N_COUNT];

    typedef struct
    {
        uchar channel_count;
        uchar data_flags;
        uchar color_type;
        uchar external_golomb_table;
        size_t image_width;
        size_t image_height;
        size_t max_bit_length;

        size_t x_block_count;
        size_t y_block_count;
    } Header;

    typedef struct FilterTypes
    {
        ulong data_size;
        std::unique_ptr<uchar> data;

        FilterTypes(IO &io) : data(nullptr)
        {
            data_size = io.read_u32_le();
            data.reset(new uchar[data_size]);
            io.read(data.get(), data_size);
        }

        ~FilterTypes()
        {
        }

        void decompress(Header &header)
        {
            size_t output_size = header.x_block_count * header.y_block_count;
            std::unique_ptr<uchar> output(new uchar[output_size]);

            LzssDecompressor decompressor;
            uchar dictionary[4096];
            uchar *ptr = dictionary;
            for (size_t i = 0; i < 32; i ++)
            {
                for (size_t j = 0; j < 16; j ++)
                {
                    for (size_t k = 0; k < 4; k ++)
                        *ptr ++ = i;

                    for (size_t k = 0; k < 4; k ++)
                        *ptr ++ = j;
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
    } FilterTypes;

    void transformer1(uchar &, uchar &, uchar &)
    {
    }

    void transformer2(uchar &r, uchar &g, uchar &b)
    {
        r += g;
        b += g;
    }

    void transformer3(uchar &r, uchar &g, uchar &b)
    {
        g += b;
        r += g;
    }

    void transformer4(uchar &r, uchar &g, uchar &b)
    {
        g += r;
        b += g;
    }

    void transformer5(uchar &r, uchar &g, uchar &b)
    {
        b += r;
        g += b;
        r += g;
    }

    void transformer6(uchar &r, uchar &g, uchar &b)
    {
        b += r;
        g += b;
    }

    void transformer7(uchar &, uchar &g, uchar &b)
    {
        b += g;
    }

    void transformer8(uchar &, uchar &g, uchar &b)
    {
        g += b;
    }

    void transformer9(uchar &r, uchar &g, uchar &)
    {
        r += g;
    }

    void transformer10(uchar &r, uchar &g, uchar &b)
    {
        r += b;
        g += r;
        b += g;
    }

    void transformer11(uchar &r, uchar &g, uchar &b)
    {
        b += r;
        g += r;
    }

    void transformer12(uchar &r, uchar &g, uchar &b)
    {
        r += b;
        g += b;
    }

    void transformer13(uchar &r, uchar &g, uchar &b)
    {
        r += b;
        g += r;
    }

    void transformer14(uchar &r, uchar &g, uchar &b)
    {
        b += g;
        r += b;
        g += r;
    }

    void transformer15(uchar &r, uchar &g, uchar &b)
    {
        g += r;
        b += g;
        r += b;
    }

    void transformer16(uchar &r, uchar &g, uchar &b)
    {
        g += (b << 1);
        r += (b << 1);
    }

    void (*transformers[16])(uchar &, uchar &, uchar &) =
    {
        &transformer1, &transformer2, &transformer3, &transformer4,
        &transformer5, &transformer6, &transformer7, &transformer8,
        &transformer9, &transformer10, &transformer11, &transformer12,
        &transformer13, &transformer14, &transformer15, &transformer16,
    };

    inline ulong make_gt_mask( ulong const a, ulong const b)
    {
        ulong tmp2 = ~b;
        ulong tmp =
            ((a & tmp2) + (((a ^ tmp2) >> 1) & 0x7f7f7f7f)) & 0x80808080;

        return ((tmp >> 7) + 0x7f7f7f7f) ^ 0x7f7f7f7f;
    }

    inline ulong packed_bytes_add( ulong const a, ulong const b)
    {
        return a + b - ((((a & b) << 1) + ((a ^ b) & 0xfefefefe)) & 0x01010100);
    }

    inline ulong med(ulong const a, ulong const b, ulong const c, ulong const v)
    {
        ulong aa_gt_bb = make_gt_mask(a, b);
        ulong a_xor_b_and_aa_gt_bb = ((a ^ b) & aa_gt_bb);
        ulong aa = a_xor_b_and_aa_gt_bb ^ a;
        ulong bb = a_xor_b_and_aa_gt_bb ^ b;
        ulong n = make_gt_mask(c, bb);
        ulong nn = make_gt_mask(aa, c);
        ulong m = ~(n | nn);
        return packed_bytes_add(
            (n & aa) | (nn & bb) | ((bb & m) - (c & m) + (aa & m)), v);
    }

    inline ulong avg(ulong const a, ulong const b, ulong const, ulong const v)
    {
        return packed_bytes_add((a & b)
            + (((a ^ b) & 0xfefefefe) >> 1)
            + ((a ^ b) & 0x01010101), v);
    }

    void init_table()
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

        int i, j, n;

        for (i = 0; i < LEADING_ZERO_TABLE_SIZE; i ++)
        {
            int cnt = 0;
            int j = 1;

            while (j != LEADING_ZERO_TABLE_SIZE && !(i & j))
            {
                j <<= 1;
                cnt ++;
            }

            cnt ++;

            if (j == LEADING_ZERO_TABLE_SIZE)
                cnt = 0;

            leading_zero_table[i] = cnt;
        }

        for (n = 0; n < GOLOMB_N_COUNT; n ++)
        {
            int a = 0;
            for (i = 0; i < 9; i ++)
            {
                for (j = 0; j < golomb_compression_table[n][i]; j ++)
                {
                    golomb_bit_length_table[a ++][n] = i;
                }
            }
        }
    }

    void decode_golomb_values(
        uchar *pixel_buf,
        int pixel_count,
        uchar *bit_pool)
    {
        int n = GOLOMB_N_COUNT - 1;
        int a = 0;

        int bit_pos = 1;
        uchar zero = (*bit_pool & 1) ? 0 : 1;
        uchar *limit = pixel_buf + pixel_count * 4;

        while (pixel_buf < limit)
        {
            int count;
            {
                ulong t = *reinterpret_cast<ulong*>(bit_pool) >> bit_pos;
                int b = leading_zero_table[t & (LEADING_ZERO_TABLE_SIZE - 1)];
                int bit_count = b;
                while (!b)
                {
                    bit_count += LEADING_ZERO_TABLE_BITS;
                    bit_pos += LEADING_ZERO_TABLE_BITS;
                    bit_pool += bit_pos >> 3;
                    bit_pos &= 7;
                    t = *reinterpret_cast<ulong*>(bit_pool) >> bit_pos;
                    b = leading_zero_table[t & (LEADING_ZERO_TABLE_SIZE - 1)];
                    bit_count += b;
                }

                bit_pos += b;
                bit_pool += bit_pos >> 3;
                bit_pos &= 7;

                bit_count --;
                count = 1 << bit_count;
                t = *reinterpret_cast<ulong*>(bit_pool);
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
                while (-- count);
            }
            else
            {
                do
                {
                    int bit_count;
                    int b;

                    ulong t = *reinterpret_cast<ulong*>(bit_pool) >> bit_pos;
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
                            t = *reinterpret_cast<ulong*>(bit_pool) >> bit_pos;
                            b = leading_zero_table[
                                t & (LEADING_ZERO_TABLE_SIZE - 1)];
                            bit_count += b;
                        }
                        bit_count --;
                    }
                    else
                    {
                        bit_pool += 5;
                        bit_count = bit_pool[-1];
                        bit_pos = 0;
                        t = *reinterpret_cast<ulong*>(bit_pool);
                        b = 0;
                    }

                    int k = golomb_bit_length_table[a][n];
                    int v = (bit_count << k) + ((t >> b) & ((1 << k) - 1));
                    int sign = (v & 1) - 1;
                    v >>= 1;
                    a += v;

                    *reinterpret_cast<uchar*>(pixel_buf)
                        = ((v ^ sign) + sign + 1);
                    pixel_buf += 4;

                    bit_pos += b;
                    bit_pos += k;
                    bit_pool += bit_pos >> 3;
                    bit_pos &= 7;

                    if (-- n < 0)
                    {
                        a >>= 1;
                        n = GOLOMB_N_COUNT - 1;
                    }
                }
                while (-- count);
            }

            zero ^= 1;
        }
    }

    void decode_line(
        ulong *prev_line,
        ulong *current_line,
        int start_block,
        int block_limit,
        uchar *filter_types,
        int skip_block_bytes,
        ulong *in,
        int odd_skip,
        int dir,
        Header &header)
    {
        ulong initialp = header.channel_count == 3 ? 0xff000000 : 0;
        ulong p, up;
        int step, i;

        if (start_block)
        {
            prev_line += start_block * W_BLOCK_SIZE;
            current_line += start_block * W_BLOCK_SIZE;
            p = current_line[-1];
            up = prev_line[-1];
        }
        else
        {
            p = up = initialp;
        }

        in += skip_block_bytes * start_block;
        step = (dir & 1) ? 1 : -1;

        for (i = start_block; i < block_limit; i ++)
        {
            int w = header.image_width - i * W_BLOCK_SIZE;
            if (w > W_BLOCK_SIZE)
                w = W_BLOCK_SIZE;

            int ww = w;
            if (step == -1)
                in += ww - 1;

            if (i & 1)
                in += odd_skip * ww;

            ulong (*filter)(
                ulong const,
                ulong const,
                ulong const,
                ulong const)
                = filter_types[i] & 1 ? &avg : &med;

            void (*transformer)(uchar &, uchar &, uchar &)
                = transformers[filter_types[i] >> 1];

            do
            {
                uchar a = (*in >> 24) & 0xff;
                uchar r = (*in >> 16) & 0xff;
                uchar g = (*in >> 8) & 0xff;
                uchar b = (*in) & 0xff;

                transformer(r, g, b);

                ulong u = *prev_line;
                p = filter(
                    p,
                    u,
                    up,
                    (0xff0000 & (b << 16))
                    + (0xff00 & (g << 8))
                    + (0xff & r) + (a << 24));

                if (header.channel_count == 3)
                    p |= 0xff000000;

                up = u;
                *current_line ++ = p;

                prev_line ++;
                in += step;
            }
            while (-- w);

            in += skip_block_bytes + (step == 1 ? - ww : 1);
            if (i & 1)
                in -= odd_skip * ww;
        }
    }

    void read_pixels(IO &io, uchar *output, Header &header)
    {
        FilterTypes filter_types(io);
        filter_types.decompress(header);

        std::unique_ptr<ulong[]> pixel_buf(
            new ulong[4 * header.image_width * H_BLOCK_SIZE]);
        std::unique_ptr<ulong[]> zero_line(new ulong[header.image_width]);
        ulong *prev_line = zero_line.get();
        for (size_t i = 0; i < header.image_width; i ++)
            zero_line[i] = 0;

        ulong main_count = header.image_width / W_BLOCK_SIZE;
        for (size_t y = 0; y < header.image_height; y += H_BLOCK_SIZE)
        {
            ulong ylim = y + H_BLOCK_SIZE;
            if (ylim >= header.image_height)
                ylim = header.image_height;

            int pixel_count = (ylim - y) * header.image_width;
            for (size_t c = 0; c < header.channel_count; c ++)
            {
                ulong bit_length = io.read_u32_le();

                int method = (bit_length >> 30) & 3;
                bit_length &= 0x3fffffff;

                int byte_length = bit_length / 8;
                if (bit_length % 8)
                    byte_length ++;

                std::unique_ptr<uchar[]> bit_pool(new uchar[byte_length]);
                io.read(bit_pool.get(), byte_length);

                if (method != 0)
                    throw std::runtime_error("Unsupported encoding method");

                decode_golomb_values(
                    reinterpret_cast<uchar*>(pixel_buf.get()) + c,
                    pixel_count,
                    bit_pool.get());
            }

            uchar *ft = filter_types.data.get()
                + (y / H_BLOCK_SIZE) * header.x_block_count;
            int skip_bytes = (ylim - y) * W_BLOCK_SIZE;

            for (size_t yy = y; yy < ylim; yy ++)
            {
                ulong *current_line = &reinterpret_cast<ulong*>(output)[
                    yy * header.image_width];

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
}

void Tlg6Decoder::decode(File &file)
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
    std::unique_ptr<uchar[]> pixels(new uchar[pixels_size]);
    for (size_t i = 0; i < pixels_size; i ++)
        pixels[i] = 0;

    read_pixels(file.io, pixels.get(), header);

    std::unique_ptr<Image> image = Image::from_pixels(
        header.image_width,
        header.image_height,
        std::string(reinterpret_cast<char*>(pixels.get()), pixels_size),
        PixelFormat::RGBA);
    image->update_file(file);
}
