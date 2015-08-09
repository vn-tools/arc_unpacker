#ifndef AU_UTIL_LZSS_H
#define AU_UTIL_LZSS_H
#include <string>
#include "io/bit_reader.h"

namespace au {
namespace util {
namespace pack {

    struct LzssSettings
    {
        size_t position_bits;
        size_t length_bits;
        size_t min_match_length;
        size_t initial_dictionary_pos;
        bool reuse_compressed;
    };

    bstr lzss_decompress(
        const bstr &input,
        size_t orig_size,
        const LzssSettings &settings);

    bstr lzss_decompress(
        io::BitReader &bit_reader,
        size_t orig_size,
        const LzssSettings &settings);

} } }

#endif
