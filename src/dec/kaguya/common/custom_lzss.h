#pragma once

#include "types.h"

namespace au {
namespace dec {
namespace kaguya {
namespace common {

    bstr custom_lzss_decompress(
        const bstr &input, const size_t size_orig);

} } } }
