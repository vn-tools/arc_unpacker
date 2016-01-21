#pragma once

#include "io/base_bit_stream.h"
#include "io/base_byte_stream.h"

namespace au {
namespace dec {
namespace wild_bug {
namespace wpx {

    struct DecoderContext final
    {
        io::BaseByteStream &input_stream;
        io::BaseBitStream &bit_stream;
    };

} } } }
