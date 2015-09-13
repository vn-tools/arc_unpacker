#pragma once

#include "types.h"

namespace au {
namespace fmt {
namespace entis {
namespace common {

    enum class Architecture : i32
    {
        RunLengthGamma   = -1,
        RunLengthHuffman = -4,
        Nemesis          = -16,
    };

    enum class Transformation : u32
    {
        Lossless = 0x03020000,
        Dct      = 0x00000001,
        Lot      = 0x00000005,
        LotMss   = 0x00000105,
    };

} } } }
