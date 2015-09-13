#pragma once

#include "fmt/entis/common/decoder.h"
#include "fmt/entis/image/common.h"

namespace au {
namespace fmt {
namespace entis {
namespace image {

    bstr decode_lossless_pixel_data(
        const EriHeader &header, common::Decoder &decoder);

} } } }
