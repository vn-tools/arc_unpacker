#pragma once

#include "types.h"

namespace au {
namespace algo {

    template<typename T> T rotl(const T what, const size_t shift)
    {
        return what << shift | (what >> ((sizeof(T) << 3) - shift));
    }

    template<typename T> T rotr(const T what, const size_t shift)
    {
        return what >> shift | (what << ((sizeof(T) << 3) - shift));
    }

} }
