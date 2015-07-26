#ifndef AU_UTIL_LZSS_H
#define AU_UTIL_LZSS_H
#include <string>
#include "io/bit_reader.h"

namespace au {
namespace util {
namespace lzss {

    typedef struct
    {
        size_t position_bits;
        size_t length_bits;
        size_t min_match_length;
        size_t initial_dictionary_pos;
        bool reuse_compressed;
    } Settings;

    std::string decompress(
        const std::string &input,
        size_t orig_size,
        const Settings &settings);

    std::string decompress(
        io::BitReader &bit_reader,
        size_t orig_size,
        const Settings &settings);

} } }

#endif
