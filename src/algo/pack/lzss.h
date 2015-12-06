#pragma once

#include <string>
#include "io/bit_reader.h"

namespace au {
namespace algo {
namespace pack {

    struct LzssSettings final
    {
        size_t position_bits;
        size_t size_bits;
        size_t min_match_size;
        size_t initial_dictionary_pos;
    };

    bstr lzss_decompress_bitwise(
        const bstr &input,
        const size_t output_size,
        const LzssSettings &settings);

    bstr lzss_decompress_bitwise(
        io::BitReader &bit_reader,
        const size_t output_size,
        const LzssSettings &settings);

    bstr lzss_decompress_bytewise(
        const bstr &input,
        const size_t output_size);

} } }
