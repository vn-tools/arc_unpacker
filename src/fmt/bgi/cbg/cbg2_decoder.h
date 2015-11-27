#pragma once

#include "io/stream.h"
#include "res/image.h"

namespace au {
namespace fmt {
namespace bgi {
namespace cbg {

    class Cbg2Decoder final
    {
    public:
        std::unique_ptr<res::Image> decode(io::Stream &input_stream) const;
    };

} } } }
