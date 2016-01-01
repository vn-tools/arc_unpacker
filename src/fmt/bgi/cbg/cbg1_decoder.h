#pragma once

#include "io/istream.h"
#include "res/image.h"

namespace au {
namespace fmt {
namespace bgi {
namespace cbg {

    class Cbg1Decoder final
    {
    public:
        std::unique_ptr<res::Image> decode(io::IStream &input_stream) const;
    };

} } } }
