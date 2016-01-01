#pragma once

#include "io/istream.h"

namespace au {
namespace fmt {
namespace liar_soft {

    void cg_decompress(
        bstr &output,
        const size_t output_offset,
        const size_t output_shift,
        io::IStream &input_stream,
        const size_t input_shift);

} } }
