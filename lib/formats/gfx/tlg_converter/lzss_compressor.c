#include "lzss_compressor.h"

void lzss_state_init(LzssState *lzss_state)
{
    lzss_state->offset = 0;
    int i;
    for (i = 0; i < 4096; i ++)
        lzss_state->dictionary[i] = 0;
}

void lzss_state_decompress(
    LzssState *lzss_state,
    unsigned char *input,
    unsigned long input_size,
    unsigned char *output,
    unsigned long output_size)
{
    unsigned char *input_guardian = input + input_size;
    unsigned char *output_guardian = output + output_size;
    int i, j;

    for (i = 0; i < output_size; i ++)
        output[i] = 0;

    int flags = 0;
    while (input < input_guardian) {
        flags >>= 1;
        if ((flags & 0x100) != 0x100) {
            if (input >= input_guardian)
                return;
            flags = *input++ | 0xff00;
        }

        if ((flags & 1) == 1) {
            if (input >= input_guardian)
                return;
            unsigned char x0 = *input++;
            if (input >= input_guardian)
                return;
            unsigned char x1 = *input++;
            int position = x0 | ((x1 & 0xf) << 8);
            int length = 3 + ((x1 & 0xf0) >> 4);
            if (length == 18) {
                if (input >= input_guardian)
                    return;
                length += *input++;
            }

            for (j = 0; j < length; j ++) {
                unsigned char c = lzss_state->dictionary[position];
                if (output >= output_guardian)
                    return;
                *output ++ = c;
                lzss_state->dictionary[lzss_state->offset] = c;
                lzss_state->offset ++;
                lzss_state->offset &= 0xfff;
                position ++;
                position &= 0xfff;
            }
        } else {
            if (input >= input_guardian)
                return;
            unsigned char c = *input ++;
            if (output >= output_guardian)
                return;
            *output ++ = c;
            lzss_state->dictionary[lzss_state->offset] = c;
            lzss_state->offset ++;
            lzss_state->offset &= 0xfff;
        }
    }
}
