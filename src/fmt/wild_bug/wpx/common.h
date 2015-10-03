#pragma once

#include "io/bit_reader.h"
#include "io/buffered_io.h"

namespace au {
namespace fmt {
namespace wild_bug {
namespace wpx {

    struct DecoderContext final
    {
        io::IO &io;
        io::BitReader &bit_reader;
    };

} } } }
