#pragma once

#include "types.h"

namespace au {
namespace fmt {
namespace fc01 {
namespace common {

    u8 rol8(const u8 x, const size_t n);

    bstr fix_stride(
        const bstr &input,
        const size_t width,
        const size_t height,
        const size_t depth);

} } } }
