#pragma once

#include <functional>
#include "types.h"

namespace au {
namespace fmt {
namespace nitroplus {

    struct NpaFilter final
    {
        const u8 *permutation;
        u32 data_key;
        std::function<u32(u32 key1, u32 key2)> file_name_key;
    };

} } }
