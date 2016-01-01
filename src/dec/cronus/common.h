#pragma once

#include "types.h"

namespace au {
namespace dec {
namespace cronus {

    u32 get_delta_key(const bstr &input);
    void delta_decrypt(bstr &input, const u32 initial_key);

} } }
