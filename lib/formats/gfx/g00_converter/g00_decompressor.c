#include <ruby.h>

static VALUE decompress_g00(
    VALUE _self,
    VALUE _input,
    VALUE _output_size,
    VALUE _byte_count,
    VALUE _length_delta) {

    unsigned char *input = (unsigned char*) RSTRING_PTR(_input);
    unsigned char *output;
    size_t input_size = RSTRING_LEN(_input);
    size_t output_size = FIX2INT(_output_size);
    size_t byte_count = FIX2INT(_byte_count);
    size_t length_delta = FIX2INT(_length_delta);

    if (input_size == 0)
        return rb_str_new("", 0);

    output = (unsigned char*) malloc(output_size);
    unsigned char *src = input;
    unsigned char *dst = output;
    unsigned char *src_guardian = input + input_size;
    unsigned char *dst_guardian = output + output_size;

    int flag = *src ++;
    int bit = 1;
    int i, look_behind, length;
    while (dst < dst_guardian) {
        if (bit == 256) {
            if (src >= src_guardian)
                break;
            flag = *src ++;
            bit = 1;
        }

        if (flag & bit) {
            for (i = 0; i < byte_count; i ++) {
                if (src >= src_guardian || dst >= dst_guardian)
                    break;
                *dst ++ = *src ++;
            }
        } else {
            if (src >= src_guardian)
                break;
            i = *src ++;
            if (src >= src_guardian)
                break;
            i |= *src ++ << 8;

            look_behind = (i >> 4) * byte_count;
            length = ((i & 0x0f) + length_delta) * byte_count;
            for (i = 0; i < length; i ++) {
                if (dst >= dst_guardian)
                    break;
                *dst = dst[-look_behind];
                dst ++;
            }
        }

        bit <<= 1;
    }

    VALUE ret = rb_str_new((char*) output, output_size);
    free(output);
    return ret;
}

void Init_g00_decompressor() {
    rb_define_global_function("decompress_g00", decompress_g00, 4);
}
