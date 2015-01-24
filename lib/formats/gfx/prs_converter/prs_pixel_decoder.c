#include <string.h>
#include "ruby.h"

VALUE prs_decode_pixels(
    VALUE _self,
    VALUE _source_buffer,
    VALUE _image_width,
    VALUE _image_height) {

    int i;
    unsigned long image_width = FIX2INT(_image_width);
    unsigned long image_height = FIX2INT(_image_height);
    unsigned long target_size = image_width * image_height * 3;
    unsigned long source_size = RSTRING_LEN(_source_buffer);
    unsigned char *source_buffer = (unsigned char*)RSTRING_PTR(_source_buffer);
    unsigned char *target_buffer = (unsigned char*)malloc(target_size);
    unsigned char *source = source_buffer;
    unsigned char *target = target_buffer;
    unsigned char *source_guardian = source + source_size;
    unsigned char *target_guardian = target + target_size;

    int flag = 0;

    int length_lookup[256];
    for (i = 0; i < 256; i ++)
        length_lookup[i] = i + 3;
    length_lookup[0xff] = 0x1000;
    length_lookup[0xfe] = 0x400;
    length_lookup[0xfd] = 0x100;

    while (1) {
        flag <<= 1;
        if ((flag & 0xff) == 0) {
            if (source >= source_guardian)
                break;
            flag = *source ++;
            flag <<= 1;
            flag += 1;
        }

        if ((flag & 0x100) != 0x100) {
            if (source >= source_guardian || target >= target_guardian)
                break;

            *target ++ = *source ++;
        } else {
            int tmp = *source ++;
            int length = 0;
            int shift = 0;

            if (tmp < 0x80) {
                length = tmp >> 2;
                tmp &= 3;
                if (tmp == 3) {
                    length += 9;
                    for (i = 0; i < length; i ++) {
                        if (source >= source_guardian || target >= target_guardian)
                            break;
                        *target ++ = *source ++;
                    }
                    continue;
                }
                shift = length;
                length = tmp + 2;
            } else {
                if (source >= source_guardian)
                    break;

                shift = (*source ++) | ((tmp & 0x3f) << 8);
                if ((tmp & 0x40) == 0) {
                    length = shift;
                    shift >>= 4;
                    length &= 0xf;
                    length += 3;
                } else {
                    if (source >= source_guardian)
                        break;

                    length = length_lookup[*source ++];
                }
            }

            shift += 1;
            for (i = 0; i < length; i ++) {
                if (target >= target_guardian)
                    break;
                *target = *(target - shift);
                target ++;
            }
        }
    }

    for (i = 3; i < target_size; i ++) {
        target_buffer[i] += target_buffer[i-3];
    }

    return rb_str_new((char*) target_buffer, target_size);
}

void Init_prs_pixel_decoder() {
    rb_define_global_function("prs_decode_pixels", prs_decode_pixels, 3);
}
