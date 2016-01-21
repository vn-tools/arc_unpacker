#pragma once

#include "io/base_byte_stream.h"

namespace au {
namespace dec {
namespace liar_soft {

    void cg_decompress(
        bstr &output,
        const size_t output_offset,
        const size_t output_shift,
        io::BaseByteStream &input_stream,
        const size_t input_shift);

} } }
