#pragma once

#include "types.h"

namespace au {
namespace fmt {
namespace glib {

    class GmlDecoder
    {
    public:
        static bstr decode(const bstr &source, size_t target_size);
    };

} } }
