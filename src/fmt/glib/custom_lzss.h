#pragma once

#include "io/stream.h"
#include "types.h"

namespace au {
namespace fmt {
namespace glib {

    bstr custom_lzss_decompress(
        const bstr &input, const size_t output_size);

    bstr custom_lzss_decompress(
        io::Stream &input_stream, const size_t output_size);

} } }
