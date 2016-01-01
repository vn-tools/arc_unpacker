#pragma once

#include "types.h"

namespace au {
namespace dec {
namespace leaf {
namespace common {

    bstr custom_lzss_decompress(const bstr &input, const size_t output_size);

} } } }
