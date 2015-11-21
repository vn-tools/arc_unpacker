#pragma once

#include "io/stream.h"

namespace au {
namespace fmt {
namespace liar_soft {

    void cg_decompress(
        bstr &output,
        const size_t output_offset,
        const size_t output_shift,
        io::Stream &input_stream,
        const size_t input_shift);

} } }
