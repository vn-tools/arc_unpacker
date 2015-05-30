#include <stdexcept>
#include "formats/kirikiri/tlg/lzss_decompressor.h"
#include "formats/kirikiri/tlg/tlg6_decoder.h"
#include "util/image.h"
using namespace Formats::Kirikiri::Tlg;

namespace
{
    const int W_BLOCK_SIZE = 8;
    const int H_BLOCK_SIZE = 8;
    const int GOLOMB_N_COUNT = 4;
    const int LEADING_ZERO_TABLE_BITS = 12;
    const int LEADING_ZERO_TABLE_SIZE = (1 << LEADING_ZERO_TABLE_BITS);

    uint8_t leading_zero_table[LEADING_ZERO_TABLE_SIZE];
    uint8_t golomb_bit_length_table[GOLOMB_N_COUNT * 2 * 128][GOLOMB_N_COUNT];

    typedef struct
    {
        uint8_t channel_count;
        uint8_t data_flags;
        uint8_t color_type;
        uint8_t external_golomb_table;
        size_t image_width;
        size_t image_height;
        size_t max_bit_length;

        size_t x_block_count;
        size_t y_block_count;
    } Header;

    typedef struct FilterTypes
    {
        uint32_t data_size;
        std::unique_ptr<uint8_t> data;

        FilterTypes(IO &io) : data(nullptr)
        {
            data_size = io.read_u32_le();
            data.reset(new uint8_t[data_size]);
            io.read(data.get(), data_size);
        }

        ~FilterTypes()
        {
        }

        void decompress(Header &header)
        {
            size_t output_size = header.x_block_count * header.y_block_count;
            std::unique_ptr<uint8_t> output(new uint8_t[output_size]);

            LzssDecompressor decompressor;
            uint8_t dictionary[4096];
            uint8_t *ptr = dictionary;
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

    void transformer1(uint8_t &, uint8_t &, uint8_t &)
    {
    }

    void transformer2(uint8_t &r, uint8_t &g, uint8_t &b)
    {
        r += g;
        b += g;
    }

    void transformer3(uint8_t &r, uint8_t &g, uint8_t &b)
    {
        g += b;
        r += g;
    }

    void transformer4(uint8_t &r, uint8_t &g, uint8_t &b)
    {
        g += r;
        b += g;
    }

    void transformer5(uint8_t &r, uint8_t &g, uint8_t &b)
    {
        b += r;
        g += b;
        r += g;
    }

    void transformer6(uint8_t &r, uint8_t &g, uint8_t &b)
    {
        b += r;
        g += b;
    }

    void transformer7(uint8_t &, uint8_t &g, uint8_t &b)
    {
        b += g;
    }

    void transformer8(uint8_t &, uint8_t &g, uint8_t &b)
    {
        g += b;
    }

    void transformer9(uint8_t &r, uint8_t &g, uint8_t &)
    {
        r += g;
    }

    void transformer10(uint8_t &r, uint8_t &g, uint8_t &b)
    {
        r += b;
        g += r;
        b += g;
    }

    void transformer11(uint8_t &r, uint8_t &g, uint8_t &b)
    {
        b += r;
        g += r;
    }

    void transformer12(uint8_t &r, uint8_t &g, uint8_t &b)
    {
        r += b;
        g += b;
    }

    void transformer13(uint8_t &r, uint8_t &g, uint8_t &b)
    {
        r += b;
        g += r;
    }

    void transformer14(uint8_t &r, uint8_t &g, uint8_t &b)
    {
        b += g;
        r += b;
        g += r;
    }

    void transformer15(uint8_t &r, uint8_t &g, uint8_t &b)
    {
        g += r;
        b += g;
        r += b;
    }

    void transformer16(uint8_t &r, uint8_t &g, uint8_t &b)
    {
        g += (b << 1);
        r += (b << 1);
    }

    void (*transformers[16])(uint8_t &, uint8_t &, uint8_t &) =
    {
        &transformer1, &transformer2, &transformer3, &transformer4,
        &transformer5, &transformer6, &transformer7, &transformer8,
        &transformer9, &transformer10, &transformer11, &transformer12,
        &transformer13, &transformer14, &transformer15, &transformer16,
    };

    inline uint32_t make_gt_mask(uint32_t a, uint32_t b)
    {
        uint32_t tmp2 = ~b;
        uint32_t tmp =
            ((a & tmp2) + (((a ^ tmp2) >> 1) & 0x7f7f7f7f)) & 0x80808080;

        return ((tmp >> 7) + 0x7f7f7f7f) ^ 0x7f7f7f7f;
    }

    inline uint32_t packed_bytes_add(uint32_t a, uint32_t b)
    {
        return a + b - ((((a & b) << 1) + ((a ^ b) & 0xfefefefe)) & 0x01010100);
    }

    inline uint32_t med(uint32_t a, uint32_t b, uint32_t c, uint32_t v)
    {
        uint32_t aa_gt_bb = make_gt_mask(a, b);
        uint32_t a_xor_b_and_aa_gt_bb = ((a ^ b) & aa_gt_bb);
        uint32_t aa = a_xor_b_and_aa_gt_bb ^ a;
        uint32_t bb = a_xor_b_and_aa_gt_bb ^ b;
        uint32_t n = make_gt_mask(c, bb);
        uint32_t nn = make_gt_mask(aa, c);
        uint32_t m = ~(n | nn);
        return packed_bytes_add(
            (n & aa) | (nn & bb) | ((bb & m) - (c & m) + (aa & m)), v);
    }

    inline uint32_t avg(uint32_t a, uint32_t b, uint32_t, uint32_t v)
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
        uint8_t *pixel_buf,
        int pixel_count,
        uint8_t *bit_pool)
    {
        int n = GOLOMB_N_COUNT - 1;
        int a = 0;

        int bit_pos = 1;
        uint8_t zero = (*bit_pool & 1) ? 0 : 1;
        uint8_t *limit = pixel_buf + pixel_count * 4;

        while (pixel_buf < limit)
        {
            int count;
            {
                uint32_t t = *reinterpret_cast<uint32_t*>(bit_pool) >> bit_pos;
                int b = leading_zero_table[t & (LEADING_ZERO_TABLE_SIZE - 1)];
                int bit_count = b;
                while (!b)
                {
                    bit_count += LEADING_ZERO_TABLE_BITS;
                    bit_pos += LEADING_ZERO_TABLE_BITS;
                    bit_pool += bit_pos >> 3;
                    bit_pos &= 7;
                    t = *reinterpret_cast<uint32_t*>(bit_pool) >> bit_pos;
                    b = leading_zero_table[t & (LEADING_ZERO_TABLE_SIZE - 1)];
                    bit_count += b;
                }

                bit_pos += b;
                bit_pool += bit_pos >> 3;
                bit_pos &= 7;

                bit_count --;
                count = 1 << bit_count;
                t = *reinterpret_cast<uint32_t*>(bit_pool);
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
                while (-- count && pixel_buf < limit);
            }
            else
            {
                do
                {
                    int bit_count;
                    int b;

                    uint32_t t = *reinterpret_cast<uint32_t*>(bit_pool);
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
                            t = *reinterpret_cast<uint32_t*>(bit_pool);
                            t >>= bit_pos;
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
                        t = *reinterpret_cast<uint32_t*>(bit_pool);
                        b = 0;
                    }

                    if (a >= GOLOMB_N_COUNT * 2 * 128) a = 0;
                    if (n >= GOLOMB_N_COUNT) n = 0;

                    int k = golomb_bit_length_table[a][n];
                    int v = (bit_count << k) + ((t >> b) & ((1 << k) - 1));
                    int sign = (v & 1) - 1;
                    v >>= 1;
                    a += v;

                    *reinterpret_cast<uint8_t*>(pixel_buf)
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
        uint32_t *prev_line,
        uint32_t *current_line,
        int start_block,
        int block_limit,
        uint8_t *filter_types,
        int skip_block_bytes,
        uint32_t *in,
        int odd_skip,
        int dir,
        Header &header)
    {
        uint32_t initialp = header.channel_count == 3 ? 0xff000000 : 0;
        uint32_t p, up;
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

            uint32_t (*filter)(uint32_t, uint32_t, uint32_t, uint32_t)
                = filter_types[i] & 1 ? &avg : &med;

            void (*transformer)(uint8_t &, uint8_t &, uint8_t &)
                = transformers[filter_types[i] >> 1];

            do
            {
                uint8_t a = (*in >> 24) & 0xff;
                uint8_t r = (*in >> 16) & 0xff;
                uint8_t g = (*in >> 8) & 0xff;
                uint8_t b = (*in) & 0xff;

                transformer(r, g, b);

                uint32_t u = *prev_line;
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

    void read_pixels(IO &io, uint8_t *output, Header &header)
    {
        FilterTypes filter_types(io);
        filter_types.decompress(header);

        std::unique_ptr<uint32_t[]> pixel_buf(
            new uint32_t[4 * header.image_width * H_BLOCK_SIZE]);
        std::unique_ptr<uint32_t[]> zero_line(new uint32_t[header.image_width]);
        uint32_t *prev_line = zero_line.get();
        for (size_t i = 0; i < header.image_width; i ++)
            zero_line[i] = 0;

        uint32_t main_count = header.image_width / W_BLOCK_SIZE;
        for (size_t y = 0; y < header.image_height; y += H_BLOCK_SIZE)
        {
            uint32_t ylim = y + H_BLOCK_SIZE;
            if (ylim >= header.image_height)
                ylim = header.image_height;

            int pixel_count = (ylim - y) * header.image_width;
            for (size_t c = 0; c < header.channel_count; c ++)
            {
                uint32_t bit_length = io.read_u32_le();

                int method = (bit_length >> 30) & 3;
                bit_length &= 0x3fffffff;

                int byte_length = bit_length / 8;
                if (bit_length % 8)
                    byte_length ++;

                std::unique_ptr<uint8_t[]> bit_pool(new uint8_t[byte_length]);
                io.read(bit_pool.get(), byte_length);

                if (method != 0)
                    throw std::runtime_error("Unsupported encoding method");

                decode_golomb_values(
                    reinterpret_cast<uint8_t*>(pixel_buf.get()) + c,
                    pixel_count,
                    bit_pool.get());
            }

            uint8_t *ft = filter_types.data.get()
                + (y / H_BLOCK_SIZE) * header.x_block_count;
            int skip_bytes = (ylim - y) * W_BLOCK_SIZE;

            for (size_t yy = y; yy < ylim; yy ++)
            {
                uint32_t *current_line
                    = &reinterpret_cast<uint32_t*>(output)
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
    std::unique_ptr<uint8_t[]> pixels(new uint8_t[pixels_size]);
    for (size_t i = 0; i < pixels_size; i ++)
        pixels[i] = 0;

    read_pixels(file.io, pixels.get(), header);

    std::unique_ptr<Image> image = Image::from_pixels(
        header.image_width,
        header.image_height,
        std::string(reinterpret_cast<char*>(pixels.get()), pixels_size),
        PixelFormat::RGBA);
    return image->create_file(file.name);
}
