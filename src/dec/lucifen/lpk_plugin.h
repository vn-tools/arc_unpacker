#pragma once

#include <map>
#include "types.h"

namespace au {
namespace dec {
namespace lucifen {

    struct LpkKey final
    {
        u32 key1;
        u32 key2;
    };

    struct LpkPlugin final
    {
        LpkKey base_key;
        u8 content_xor;
        u32 rotate_pattern;
        std::map<std::string, LpkKey> file_keys;
    };

} } }
