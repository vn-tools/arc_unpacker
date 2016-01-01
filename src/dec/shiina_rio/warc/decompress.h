#pragma once

#include "types.h"

namespace au {
namespace dec {
namespace shiina_rio {
namespace warc {

    bstr decompress_yh1(
        const bstr &input, const size_t size_orig, const bool encrypted);

    bstr decompress_ylz(
        const bstr &input, const size_t size_orig, const bool encrypted);

    bstr decompress_ypk(
        const bstr &input, const size_t size_orig, const bool encrypted);

} } } }
