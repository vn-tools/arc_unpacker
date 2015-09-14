#pragma once

#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace nitroplus {

    struct NpaFilter final
    {
        const u8 *permutation;
        u32 data_key;
        u32 (*file_name_key)(u32 key1, u32 key2);
    };

} } }
