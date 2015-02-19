#ifndef STRING_LZSS_H
#define STRING_LZSS_H
#include <string>
#include "bit_reader.h"

typedef struct
{
    size_t position_bits;
    size_t length_bits;
    size_t min_match_length;
    size_t initial_dictionary_pos;
    bool reuse_compressed;
} LzssSettings;

std::string lzss_decompress(
    BitReader &bit_reader,
    size_t size_original,
    const LzssSettings &settings);

#endif
