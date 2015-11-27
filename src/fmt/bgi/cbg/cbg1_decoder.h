#pragma once

#include "io/stream.h"
#include "pix/image.h"

namespace au {
namespace fmt {
namespace bgi {
namespace cbg {

    class Cbg1Decoder final
    {
    public:
        std::unique_ptr<pix::Image> decode(io::Stream &input_stream) const;
    };

} } } }
