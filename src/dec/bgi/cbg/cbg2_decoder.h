#pragma once

#include "io/base_byte_stream.h"
#include "res/image.h"

namespace au {
namespace dec {
namespace bgi {
namespace cbg {

    class Cbg2Decoder final
    {
    public:
        std::unique_ptr<res::Image> decode(
            io::BaseByteStream &input_stream) const;
    };

} } } }
