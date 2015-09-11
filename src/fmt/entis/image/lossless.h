#pragma once

#include "fmt/entis/common/decoder.h"
#include "fmt/entis/image/common.h"

namespace au {
namespace fmt {
namespace entis {
namespace image {

    bstr decode_lossless_pixel_data(
        const EriHeader &header,
        const bstr &encoded_pixel_data,
        common::Decoder &decoder);

} } } }
