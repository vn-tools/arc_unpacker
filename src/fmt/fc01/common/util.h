#pragma once

#include "types.h"

namespace au {
namespace fmt {
namespace fc01 {
namespace common {

    u8 rol8(u8 x, size_t n);

    bstr fix_stride(
        const bstr &input, size_t width, size_t height, size_t depth);

} } } }
