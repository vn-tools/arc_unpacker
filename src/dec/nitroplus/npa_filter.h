#pragma once

#include <functional>
#include "types.h"

namespace au {
namespace dec {
namespace nitroplus {

    struct NpaFilter final
    {
        const bstr permutation;
        const u32 data_key;
        std::function<u32(const u32 key1, const u32 key2)> file_name_key;
    };

} } }
