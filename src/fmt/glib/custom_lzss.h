#pragma once

#include "io/istream.h"
#include "types.h"

namespace au {
namespace fmt {
namespace glib {

    bstr custom_lzss_decompress(
        const bstr &input, const size_t output_size);

    bstr custom_lzss_decompress(
        io::IStream &input_stream, const size_t output_size);

} } }
