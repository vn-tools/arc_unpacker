#pragma once

#include "io/io.h"
#include "types.h"

namespace au {
namespace fmt {
namespace glib {

    class GmlDecoder final
    {
    public:
        static bstr decode(const bstr &input, size_t output_size);
        static bstr decode(io::IO &input_io, size_t output_size);
    };

} } }
