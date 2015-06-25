#ifndef UTIL_LZSS_H
#define UTIL_LZSS_H
#include <string>
#include "io/bit_reader.h"

typedef struct
{
    size_t position_bits;
    size_t length_bits;
    size_t min_match_length;
    size_t initial_dictionary_pos;
    bool reuse_compressed;
} LzssSettings;

std::string lzss_decompress(
    const std::string &input, size_t orig_size, const LzssSettings &settings);

std::string lzss_decompress(
    BitReader &bit_reader, size_t orig_size, const LzssSettings &settings);

#endif
