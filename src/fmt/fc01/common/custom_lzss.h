#pragma once

#include "io/stream.h"

namespace au {
namespace fmt {
namespace fc01 {
namespace common {

    bstr custom_lzss_decompress(const bstr &input, size_t output_size);
    bstr custom_lzss_decompress(io::Stream &input_stream, size_t output_size);

} } } }
