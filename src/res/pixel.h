#pragma once

#include "types.h"

namespace au {
namespace res {

    struct Pixel final
    {
        u8 b, g, r, a;

        constexpr bool operator ==(const Pixel &other) const
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        constexpr bool operator !=(const Pixel &other) const
        {
            return r != other.r || g != other.g || b != other.b || a != other.a;
        }

        inline const u8 &operator[](size_t x) const
        {
            return reinterpret_cast<const u8*>(this)[x];
        }

        inline u8 &operator[](size_t x)
        {
            return reinterpret_cast<u8*>(this)[x];
        }
    };

} }
