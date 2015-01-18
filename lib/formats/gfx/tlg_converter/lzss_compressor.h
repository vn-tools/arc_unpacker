#ifndef LZSS_COMPRESSOR_H
#define LZSS_COMPRESSOR_H

typedef struct
{
    unsigned char dictionary[4096];
    unsigned int offset;
} LzssState;

void lzss_state_init(LzssState *lzss_state);

void lzss_state_decompress(
    LzssState *lzss_state,
    unsigned char *input,
    unsigned long input_size,
    unsigned char *output,
    unsigned long output_size);

#endif
