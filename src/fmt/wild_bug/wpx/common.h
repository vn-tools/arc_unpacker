#pragma once

#include "io/bit_reader.h"
#include "io/memory_stream.h"

namespace au {
namespace fmt {
namespace wild_bug {
namespace wpx {

    struct DecoderContext final
    {
        io::Stream &stream;
        io::IBitReader &bit_reader;
    };

} } } }
