#pragma once

#include "io/io.h"

namespace au {
namespace fmt {
namespace liar_soft {

    void cg_decompress(
        bstr &output,
        const size_t output_offset,
        const size_t output_shift,
        io::IO &input_io,
        const size_t input_shift);

} } }
