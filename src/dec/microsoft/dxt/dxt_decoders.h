#pragma once

#include "io/base_byte_stream.h"
#include "res/image.h"

namespace au {
namespace dec {
namespace microsoft {
namespace dxt {

    std::unique_ptr<res::Image> decode_dxt1(
        io::BaseByteStream &input_stream,
        const size_t width,
        const size_t height);

    std::unique_ptr<res::Image> decode_dxt3(
        io::BaseByteStream &input_stream,
        const size_t width,
        const size_t height);

    std::unique_ptr<res::Image> decode_dxt5(
        io::BaseByteStream &input_stream,
        const size_t width,
        const size_t height);

} } } }
