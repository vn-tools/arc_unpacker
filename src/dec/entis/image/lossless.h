#pragma once

#include "dec/entis/common/abstract_decoder.h"
#include "dec/entis/image/common.h"

namespace au {
namespace dec {
namespace entis {
namespace image {

    bstr decode_lossless_pixel_data(
        const EriHeader &header, common::AbstractDecoder &decoder);

} } } }
