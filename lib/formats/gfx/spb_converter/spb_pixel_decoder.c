#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ruby.h>
#include "../../../bit_stream.c" //TODO: relative paths have to do for now

VALUE decode_spb_pixels(
    VALUE _self,
    VALUE _input,
    VALUE _image_width,
    VALUE _image_height) {

    size_t input_size = RSTRING_LEN(_input);
    unsigned char *input = (unsigned char*)RSTRING_PTR(_input);
    unsigned char *output;
    unsigned char *tmp_data;
    unsigned short w = FIX2INT(_image_width);
    unsigned short h = FIX2INT(_image_height);
    size_t output_size;
    size_t tmp_data_size;

    unsigned char *p, *q;
    unsigned char ch;
    int i, j;
    int t, mask;
    int rgb;

    if (input_size < 4)
        return 0;

    output_size = w * h * 3;
    output = (unsigned char*) malloc(output_size);

    tmp_data_size = w * h;
    tmp_data = (unsigned char*) malloc(tmp_data_size);

    bit_stream bs;
    bit_stream_init(&bs, input, input_size);

    unsigned char *dest_guardian = tmp_data + tmp_data_size;
    unsigned char *dest = tmp_data;
    for (rgb = 2; rgb >= 0; rgb--, dest = tmp_data) {
        ch = bit_stream_get(&bs, 8);
        if (dest >= dest_guardian)
            break;
        *dest ++ = ch;

        while (dest < tmp_data + tmp_data_size) {
            t = bit_stream_get(&bs, 3);
            if (t == 0) {
                if (dest >= dest_guardian)
                    break;
                *dest ++ = ch;
                if (dest >= dest_guardian)
                    break;
                *dest ++ = ch;
                if (dest >= dest_guardian)
                    break;
                *dest ++ = ch;
                if (dest >= dest_guardian)
                    break;
                *dest ++ = ch;
                continue;
            }

            mask = t == 7
                ? bit_stream_get(&bs, 1) + 1
                : t + 2;

            for (i = 0; i < 4; i++) {
                if (mask == 8) {
                    ch = bit_stream_get(&bs, 8);
                } else {
                    t = bit_stream_get(&bs, mask);
                    if (t & 1) {
                        ch += (t >> 1) + 1;
                    } else {
                        ch -= (t >> 1);
                    }
                }
                if (dest >= dest_guardian)
                    break;
                *dest ++ = ch;
            }
        }

        p = tmp_data;
        q = output + rgb;
        for (j = 0; j < h >> 1; j++) {
            for (i = 0; i < w; i++) {
                *q = *p++;
                q += 3;
            }
            q += w * 3;
            for (i = 0; i < w; i++) {
                q -= 3;
                *q = *p++;
            }
            q += w * 3;
        }
        if (h & 1) {
            for (i = 0; i < w; i++) {
                *q = *p++;
                q += 3;
            }
        }
    }

    free(tmp_data);
    VALUE ret = rb_str_new((char*) output, output_size);
    free(output);
    return ret;
}

void Init_spb_pixel_decoder() {
    rb_define_global_function("spb_decode_pixels", decode_spb_pixels, 3);
}
