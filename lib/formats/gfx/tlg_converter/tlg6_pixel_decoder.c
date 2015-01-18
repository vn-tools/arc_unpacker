#include "tlg6_pixel_decoder.h"
#include "lzss_compressor.h"

#define TLG6_W_BLOCK_SIZE 8
#define TLG6_H_BLOCK_SIZE 8
#define TLG6_GOLOMB_N_COUNT 4
#define TLG6_LEADING_ZERO_TABLE_BITS 12
#define TLG6_LEADING_ZERO_TABLE_SIZE (1 << TLG6_LEADING_ZERO_TABLE_BITS)

static unsigned char tlg6_leading_zero_table[TLG6_LEADING_ZERO_TABLE_SIZE];
static unsigned char tlg6_golomb_bit_length_table[TLG6_GOLOMB_N_COUNT * 2 * 128][TLG6_GOLOMB_N_COUNT];

typedef struct {
    unsigned char channel_count;
    unsigned char data_flags;
    unsigned char color_type;
    unsigned char external_golomb_table;
    unsigned long image_width;
    unsigned long image_height;
    unsigned long max_bit_length;

    unsigned long x_block_count;
    unsigned long y_block_count;
} Tlg6Header;

typedef struct {
    unsigned long data_size;
    unsigned char *data;
} Tlg6FilterTypes;

static void tlg6_transformer1(unsigned char *r, unsigned char *g, unsigned char *b) {
}

static void tlg6_transformer2(unsigned char *r, unsigned char *g, unsigned char *b) {
    *r += *g;
    *b += *g;
}

static void tlg6_transformer3(unsigned char *r, unsigned char *g, unsigned char *b) {
    *g += *b;
    *r += *g;
}

static void tlg6_transformer4(unsigned char *r, unsigned char *g, unsigned char *b) {
    *g += *r;
    *b += *g;
}

static void tlg6_transformer5(unsigned char *r, unsigned char *g, unsigned char *b) {
    *b += *r;
    *g += *b;
    *r += *g;
}

static void tlg6_transformer6(unsigned char *r, unsigned char *g, unsigned char *b) {
    *b += *r;
    *g += *b;
}

static void tlg6_transformer7(unsigned char *r, unsigned char *g, unsigned char *b) {
    *b += *g;
}

static void tlg6_transformer8(unsigned char *r, unsigned char *g, unsigned char *b) {
    *g += *b;
}

static void tlg6_transformer9(unsigned char *r, unsigned char *g, unsigned char *b) {
    *r += *g;
}

static void tlg6_transformer10(unsigned char *r, unsigned char *g, unsigned char *b) {
    *r += *b;
    *g += *r;
    *b += *g;
}

static void tlg6_transformer11(unsigned char *r, unsigned char *g, unsigned char *b) {
    *b += *r;
    *g += *r;
}

static void tlg6_transformer12(unsigned char *r, unsigned char *g, unsigned char *b) {
    *r += *b;
    *g += *b;
}

static void tlg6_transformer13(unsigned char *r, unsigned char *g, unsigned char *b) {
    *r += *b;
    *g += *r;
}

static void tlg6_transformer14(unsigned char *r, unsigned char *g, unsigned char *b) {
    *b += *g;
    *r += *b;
    *g += *r;
}

static void tlg6_transformer15(unsigned char *r, unsigned char *g, unsigned char *b) {
    *g += *r;
    *b += *g;
    *r += *b;
}

static void tlg6_transformer16(unsigned char *r, unsigned char *g, unsigned char *b) {
    *g += (*b << 1);
    *r += (*b << 1);
}

static void (*transformers[16])(unsigned char *, unsigned char *, unsigned char *) = {
    &tlg6_transformer1, &tlg6_transformer2, &tlg6_transformer3, &tlg6_transformer4,
    &tlg6_transformer5, &tlg6_transformer6, &tlg6_transformer7, &tlg6_transformer8,
    &tlg6_transformer9, &tlg6_transformer10, &tlg6_transformer11, &tlg6_transformer12,
    &tlg6_transformer13, &tlg6_transformer14, &tlg6_transformer15, &tlg6_transformer16,
};

static void tlg6_filter_types_read(Tlg6FilterTypes *filter_types, unsigned char **_input) {
    unsigned char *input = *_input;

    filter_types->data_size = *(unsigned long*)input;
    input += 4;

    filter_types->data = (unsigned char*) malloc(filter_types->data_size);
    unsigned char *tmp = filter_types->data;
    int i;
    for (i = 0; i < filter_types->data_size; i ++) {
        *tmp ++ = *input ++;
    }

    *_input = input;
}

static void tlg6_filter_types_destroy(Tlg6FilterTypes *filter_types) {
    free(filter_types->data);
}

static void tlg6_filter_types_decompress(Tlg6FilterTypes *filter_types, Tlg6Header *header) {
    int output_size = header->x_block_count * header->y_block_count;
    unsigned char *output = (unsigned char*)malloc(output_size);

    LzssState lzss_state;
    lzss_state_init(&lzss_state);
    unsigned char *ptr = lzss_state.dictionary;
    int i, j, k;

    for (i = 0; i < 32; i ++) {
        for (j = 0; j < 16; j ++) {
            for (k = 0; k < 4; k ++)
                *ptr ++ = i;

            for (k = 0; k < 4; k ++)
                *ptr ++ = j;
        }
    }

    lzss_state_decompress(
        &lzss_state,
        filter_types->data,
        filter_types->data_size,
        output,
        output_size);

    free(filter_types->data);
    filter_types->data = output;
    filter_types->data_size = output_size;
}

inline unsigned long tlg6_make_gt_mask(
    unsigned long const a,
    unsigned long const b) {

    unsigned long tmp2 = ~b;
    unsigned long tmp =
        ((a & tmp2) + (((a ^ tmp2) >> 1) & 0x7f7f7f7f)) & 0x80808080;

    return ((tmp >> 7) + 0x7f7f7f7f) ^ 0x7f7f7f7f;
}

inline unsigned long tlg6_packed_bytes_add(
    unsigned long const a,
    unsigned long const b) {

    return a + b - ((((a & b) << 1) + ((a ^ b) & 0xfefefefe)) & 0x01010100);
}

inline unsigned long tlg6_med(
    unsigned long const a,
    unsigned long const b,
    unsigned long const c,
    unsigned long const v) {

    unsigned long aa_gt_bb = tlg6_make_gt_mask(a, b);
    unsigned long a_xor_b_and_aa_gt_bb = ((a ^ b) & aa_gt_bb);
    unsigned long aa = a_xor_b_and_aa_gt_bb ^ a;
    unsigned long bb = a_xor_b_and_aa_gt_bb ^ b;
    unsigned long n = tlg6_make_gt_mask(c, bb);
    unsigned long nn = tlg6_make_gt_mask(aa, c);
    unsigned long m = ~(n | nn);
    return tlg6_packed_bytes_add((n & aa) | (nn & bb) | ((bb & m) - (c & m) + (aa & m)), v);
}

inline unsigned long tlg6_avg(
    unsigned long const a,
    unsigned long const b,
    unsigned long const c,
    unsigned long const v) {

    return tlg6_packed_bytes_add((a & b)
        + (((a ^ b) & 0xfefefefe) >> 1)
        + ((a ^ b) & 0x01010101), v);
}

void tlg6_init_table() {
    short golomb_compression_table[TLG6_GOLOMB_N_COUNT][9] = {
        {3, 7, 15, 27, 63, 108, 223, 448, 130, },
        {3, 5, 13, 24, 51, 95, 192, 384, 257, },
        {2, 5, 12, 21, 39, 86, 155, 320, 384, },
        {2, 3, 9, 18, 33, 61, 129, 258, 511, },
    };

    int i, j, n;

    for (i = 0; i < TLG6_LEADING_ZERO_TABLE_SIZE; i ++) {
        int cnt = 0;
        int j = 1;

        while (j != TLG6_LEADING_ZERO_TABLE_SIZE && !(i & j)) {
            j <<= 1;
            cnt ++;
        }

        cnt ++;

        if (j == TLG6_LEADING_ZERO_TABLE_SIZE)
            cnt = 0;

        tlg6_leading_zero_table[i] = cnt;
    }

    for (n = 0; n < TLG6_GOLOMB_N_COUNT; n ++) {
        int a = 0;
        for (i = 0; i < 9; i ++) {
            for (j = 0; j < golomb_compression_table[n][i]; j ++) {
                tlg6_golomb_bit_length_table[a ++][n] = i;
            }
        }
    }
}

static void tlg6_decode_golomb_values(
    unsigned char *pixel_buf,
    int pixel_count,
    unsigned char *bit_pool) {

    int n = TLG6_GOLOMB_N_COUNT - 1;
    int a = 0;

    int bit_pos = 1;
    unsigned char zero = (*bit_pool & 1) ? 0 : 1;
    unsigned char *limit = pixel_buf + pixel_count * 4;

    while (pixel_buf < limit) {
        int count;
        {
            unsigned long t = *(unsigned long*)(bit_pool) >> bit_pos;
            int b = tlg6_leading_zero_table[t & (TLG6_LEADING_ZERO_TABLE_SIZE - 1)];
            int bit_count = b;
            while (!b) {
                bit_count += TLG6_LEADING_ZERO_TABLE_BITS;
                bit_pos += TLG6_LEADING_ZERO_TABLE_BITS;
                bit_pool += bit_pos >> 3;
                bit_pos &= 7;
                t = *(unsigned long*)(bit_pool) >> bit_pos;
                b = tlg6_leading_zero_table[t & (TLG6_LEADING_ZERO_TABLE_SIZE - 1)];
                bit_count += b;
            }

            bit_pos += b;
            bit_pool += bit_pos >> 3;
            bit_pos &= 7;

            bit_count --;
            count = 1 << bit_count;
            t = *(unsigned long*)(bit_pool);
            count += ((t >> bit_pos) & (count - 1));

            bit_pos += bit_count;
            bit_pool += bit_pos >> 3;
            bit_pos &= 7;
        }

        if (zero) {
            do {
                *pixel_buf = 0;
                pixel_buf += 4;
            }
            while (-- count);
        } else {
            do {
                int bit_count;
                int b;

                unsigned long t = *(unsigned long*)(bit_pool) >> bit_pos;
                if (t) {
                    b = tlg6_leading_zero_table[t & (TLG6_LEADING_ZERO_TABLE_SIZE - 1)];
                    bit_count = b;
                    while (!b) {
                        bit_count += TLG6_LEADING_ZERO_TABLE_BITS;
                        bit_pos += TLG6_LEADING_ZERO_TABLE_BITS;
                        bit_pool += bit_pos >> 3;
                        bit_pos &= 7;
                        t = *(unsigned long*)(bit_pool) >> bit_pos;
                        b = tlg6_leading_zero_table[t & (TLG6_LEADING_ZERO_TABLE_SIZE - 1)];
                        bit_count += b;
                    }
                    bit_count --;
                } else {
                    bit_pool += 5;
                    bit_count = bit_pool[-1];
                    bit_pos = 0;
                    t = *(unsigned long*)(bit_pool);
                    b = 0;
                }

                int k = tlg6_golomb_bit_length_table[a][n];
                int v = (bit_count << k) + ((t >> b) & ((1 << k) - 1));
                int sign = (v & 1) - 1;
                v >>= 1;
                a += v;

                *(unsigned char*)pixel_buf = ((v ^ sign) + sign + 1);
                pixel_buf += 4;

                bit_pos += b;
                bit_pos += k;
                bit_pool += bit_pos >> 3;
                bit_pos &= 7;

                if (-- n < 0) {
                    a >>= 1;
                    n = TLG6_GOLOMB_N_COUNT - 1;
                }
            }
            while (-- count);
        }

        zero ^= 1;
    }
}

static void tlg6_decode_line(
    unsigned long *prev_line,
    unsigned long *current_line,
    int start_block,
    int block_limit,
    unsigned char *filter_types,
    int skip_block_bytes,
    unsigned long *in,
    int odd_skip,
    int dir,
    Tlg6Header *header) {

    unsigned long initialp = header->channel_count == 3 ? 0xff000000 : 0;
    unsigned long p, up;
    int step, i;

    if (start_block) {
        prev_line += start_block * TLG6_W_BLOCK_SIZE;
        current_line += start_block * TLG6_W_BLOCK_SIZE;
        p = current_line[-1];
        up = prev_line[-1];
    } else {
        p = up = initialp;
    }

    in += skip_block_bytes * start_block;
    step = (dir & 1) ? 1 : -1;

    for (i = start_block; i < block_limit; i ++) {
        int w = header->image_width - i * TLG6_W_BLOCK_SIZE;
        if (w > TLG6_W_BLOCK_SIZE)
            w = TLG6_W_BLOCK_SIZE;

        int ww = w;
        if (step == -1)
            in += ww - 1;

        if (i & 1)
            in += odd_skip * ww;

        unsigned long (*filter)(
            unsigned long const,
            unsigned long const,
            unsigned long const,
            unsigned long const)
            = filter_types[i] & 1 ? &tlg6_avg : &tlg6_med;

        void (*transformer)(unsigned char *, unsigned char *, unsigned char *)
            = transformers[filter_types[i] >> 1];

        do {
            unsigned char a = (*in >> 24) & 0xff;
            unsigned char r = (*in >> 16) & 0xff;
            unsigned char g = (*in >> 8) & 0xff;
            unsigned char b = (*in) & 0xff;

            transformer(&r, &g, &b);

            unsigned long u = *prev_line;
            p = filter(
                p,
                u,
                up,
                (0xff0000 & (b << 16))
                + (0xff00 & (g << 8))
                + (0xff & r) + (a << 24));

            if (header->channel_count == 3)
                p |= 0xff000000;

            up = u;
            *current_line = p;

            current_line ++;
            prev_line ++;
            in += step;
        }
        while (-- w);

        in += skip_block_bytes + (step == 1 ? - ww : 1);
        if (i & 1)
            in -= odd_skip * ww;
    }
}

static void tlg6_read_pixels(unsigned char *input, unsigned char *output, Tlg6Header *header) {
    Tlg6FilterTypes filter_types;
    tlg6_filter_types_read(&filter_types, &input);
    tlg6_filter_types_decompress(&filter_types, header);

    int i, y, yy, c;

    unsigned long *pixel_buf = (unsigned long*)malloc(4 * header->image_width * TLG6_H_BLOCK_SIZE * sizeof(unsigned long));
    unsigned long *zero_line = (unsigned long*)malloc(header->image_width * sizeof(unsigned long));
    unsigned long *prev_line = zero_line;
    for (i = 0; i < header->image_width; i ++)
        zero_line[i] = 0;

    unsigned long main_count = header->image_width / TLG6_W_BLOCK_SIZE;
    int fraction = header->image_width - main_count * TLG6_W_BLOCK_SIZE;
    for (y = 0; y < header->image_height; y += TLG6_H_BLOCK_SIZE) {
        unsigned long ylim = y + TLG6_H_BLOCK_SIZE;
        if (ylim >= header->image_height)
            ylim = header->image_height;

        int pixel_count = (ylim - y) * header->image_width;
        for (c = 0; c < header->channel_count; c ++) {
            unsigned long bit_length = *(unsigned long*)input;
            input += 4;

            int method = (bit_length >> 30) & 3;
            bit_length &= 0x3fffffff;

            int byte_length = bit_length / 8;
            if (bit_length % 8)
                byte_length ++;

            unsigned char *bit_pool = input;
            input += byte_length;

            if (method == 0) {
                tlg6_decode_golomb_values((unsigned char*)pixel_buf + c, pixel_count, bit_pool);
            } else {
                rb_raise(rb_eRuntimeError, "Unsupported encoding method");
            }
        }

        unsigned char *ft = filter_types.data + (y / TLG6_H_BLOCK_SIZE) * header->x_block_count;
        int skip_bytes = (ylim - y) * TLG6_W_BLOCK_SIZE;

        for (yy = y; yy < ylim; yy ++) {
            unsigned long *current_line = &((unsigned long*)output)[yy * header->image_width];

            int dir = (yy & 1) ^ 1;
            int odd_skip = ((ylim - yy -1) - (yy - y));

            if (main_count) {
                int start = ((header->image_width < TLG6_W_BLOCK_SIZE)
                    ? header->image_width
                    : TLG6_W_BLOCK_SIZE) * (yy - y);

                tlg6_decode_line(
                    prev_line,
                    current_line,
                    0,
                    main_count,
                    ft,
                    skip_bytes,
                    pixel_buf + start,
                    odd_skip,
                    dir,
                    header);
            }

            if (main_count != header->x_block_count) {
                int ww = fraction;
                if (ww > TLG6_W_BLOCK_SIZE)
                    ww = TLG6_W_BLOCK_SIZE;

                int start = ww * (yy - y);
                tlg6_decode_line(
                    prev_line,
                    current_line,
                    main_count,
                    header->x_block_count,
                    ft,
                    skip_bytes,
                    pixel_buf + start,
                    odd_skip,
                    dir,
                    header);
            }

            prev_line = current_line;
        }
    }

    tlg6_filter_types_destroy(&filter_types);
}

VALUE decode_tlg6_pixels(VALUE _self, VALUE _header, VALUE _input) {
    unsigned char *input = RSTRING_PTR(_input);

    Tlg6Header header;
    header.channel_count = FIX2INT(rb_hash_aref(_header, ID2SYM(rb_intern("channel_count"))));
    header.data_flags = FIX2INT(rb_hash_aref(_header, ID2SYM(rb_intern("data_flags"))));
    header.color_type = FIX2INT(rb_hash_aref(_header, ID2SYM(rb_intern("color_type"))));
    header.external_golomb_table = FIX2INT(rb_hash_aref(_header, ID2SYM(rb_intern("external_golomb_table"))));
    header.image_width = FIX2INT(rb_hash_aref(_header, ID2SYM(rb_intern("image_width"))));
    header.image_height = FIX2INT(rb_hash_aref(_header, ID2SYM(rb_intern("image_height"))));
    header.max_bit_length = FIX2INT(rb_hash_aref(_header, ID2SYM(rb_intern("max_bit_length"))));

    header.x_block_count = ((header.image_width - 1) / TLG6_W_BLOCK_SIZE) + 1;
    header.y_block_count = ((header.image_height - 1) / TLG6_H_BLOCK_SIZE) + 1;
    if (header.channel_count != 1 && header.channel_count != 3 && header.channel_count != 4) {
        rb_raise(rb_eRuntimeError, "Unsupported channel count");
    }

    size_t output_size = header.image_width * header.image_height * 4;
    unsigned char *output = (unsigned char*)malloc(output_size);

    int i;
    for (i = 0; i < output_size; i ++)
        output[i] = 0;

    tlg6_read_pixels(input, output, &header);

    VALUE ret = rb_str_new((char*) output, output_size);
    free(output);
    return ret;
}
