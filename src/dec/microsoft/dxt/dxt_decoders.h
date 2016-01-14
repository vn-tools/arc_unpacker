#pragma once

#include "io/istream.h"
#include "res/image.h"

namespace au {
namespace dec {
namespace microsoft {
namespace dxt {

    std::unique_ptr<res::Image> decode_dxt1(
        io::IStream &input_stream, const size_t width, const size_t height);

    std::unique_ptr<res::Image> decode_dxt3(
        io::IStream &input_stream, const size_t width, const size_t height);

    std::unique_ptr<res::Image> decode_dxt5(
        io::IStream &input_stream, const size_t width, const size_t height);

} } } }
