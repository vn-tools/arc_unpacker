#pragma once

#include "types.h"

namespace au {
namespace fmt {
namespace glib {

    class GmlDecoder final
    {
    public:
        static bstr decode(const bstr &source, size_t target_size);
    };

} } }
