#pragma once

#include "types.h"

namespace au {
namespace fmt {
namespace cronus {

    u32 get_delta_key(const bstr &input);
    void delta_decrypt(bstr &buffer, u32 initial_key);

} } }
