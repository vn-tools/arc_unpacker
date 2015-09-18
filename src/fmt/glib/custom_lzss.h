#pragma once

#include "io/io.h"
#include "types.h"

namespace au {
namespace fmt {
namespace glib {

    bstr custom_lzss_decompress(const bstr &input, size_t output_size);
    bstr custom_lzss_decompress(io::IO &input_io, size_t output_size);

} } }
