#pragma once

#include "io/ibit_reader.h"
#include "io/istream.h"

namespace au {
namespace fmt {
namespace wild_bug {
namespace wpx {

    struct DecoderContext final
    {
        io::IStream &input_stream;
        io::IBitReader &bit_reader;
    };

} } } }
