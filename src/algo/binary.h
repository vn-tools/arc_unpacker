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

    inline u64 padb(const u64 a, const u64 b)
    {
        return ((a & 0x7F7F7F7F7F7F7F7F)
            + (b & 0x7F7F7F7F7F7F7F7F))
            ^ ((a ^ b) & 0x8080808080808080);
    }

    inline u64 padw(const u64 a, const u64 b)
    {
        return ((a & 0x7FFF7FFF7FFF7FFF)
            + (b & 0x7FFF7FFF7FFF7FFF))
            ^ ((a ^ b) & 0x8000800080008000);
    }

    inline u64 padd(const u64 a, const u64 b)
    {
        return ((a & 0x7FFFFFFF7FFFFFFF)
            + (b & 0x7FFFFFFF7FFFFFFF))
            ^ ((a ^ b) & 0x8000000080000000);
    }

    bstr unxor(const bstr &input, const u8 key);
    bstr unxor(const bstr &input, const bstr &key);

} }
