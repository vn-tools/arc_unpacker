#pragma once

#include <string>
#include "io/base_bit_stream.h"

namespace au {
namespace algo {
namespace pack {

    struct BitwiseLzssSettings final
    {
        size_t position_bits;
        size_t size_bits;
        size_t min_match_size;
        size_t initial_dictionary_pos;
    };

    struct BytewiseLzssSettings final
    {
        BytewiseLzssSettings();

        size_t initial_dictionary_pos;
    };

    bstr lzss_decompress(
        const bstr &input,
        const size_t output_size,
        const BitwiseLzssSettings &settings);

    bstr lzss_decompress(
        io::BaseBitStream &input_stream,
        const size_t output_size,
        const BitwiseLzssSettings &settings);

    bstr lzss_decompress(
        const bstr &input,
        const size_t output_size,
        const BytewiseLzssSettings &settings = BytewiseLzssSettings());

    bstr lzss_compress(
        const bstr &input, const algo::pack::BitwiseLzssSettings &settings);

    bstr lzss_compress(
        io::BaseByteStream &input_stream,
        const BitwiseLzssSettings &settings);

    bstr lzss_compress(
        const bstr &input,
        const BytewiseLzssSettings &settings = BytewiseLzssSettings());

    bstr lzss_compress(
        io::BaseByteStream &input_stream,
        const BytewiseLzssSettings &settings);


} } }
