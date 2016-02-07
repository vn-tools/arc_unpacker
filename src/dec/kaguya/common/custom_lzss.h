#pragma once

#include "dec/base_image_decoder.h"

namespace au {
namespace dec {
namespace kaguya {
namespace common {

    bstr custom_lzss_decompress(
        const bstr &input, const size_t size_orig);

} } } }
