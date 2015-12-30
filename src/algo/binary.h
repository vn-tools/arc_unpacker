#pragma once

#include "types.h"

namespace au {
namespace algo {

    template<typename T> T rotl(const T what, size_t shift)
    {
        shift &= (sizeof(T) << 3) - 1;
        return what << shift | (what >> ((sizeof(T) << 3) - shift));
    }

    template<typename T> T rotr(const T what, size_t shift)
    {
        shift &= (sizeof(T) << 3) - 1;
        return what >> shift | (what << ((sizeof(T) << 3) - shift));
    }

    bstr unxor(const bstr &input, const u8 key);
    bstr unxor(const bstr &input, const bstr &key);

} }
