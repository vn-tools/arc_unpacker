#pragma once

#include "types.h"

namespace au {
namespace dec {
namespace kaguya {
namespace common {

    bstr decompress_rle(
        const bstr &input, const size_t size_orig, const size_t bands);

} } } }
