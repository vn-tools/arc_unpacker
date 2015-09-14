#pragma once

#include <memory>
#include <type_traits>
#include "types.h"

namespace au {
namespace pix {

    struct Pixel final
    {
        u8 b, g, r, a;

        constexpr bool operator ==(const Pixel &other)
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        constexpr bool operator !=(const Pixel &other)
        {
            return !(operator ==(other));
        }

        constexpr const u8 &operator[](size_t x) const
        {
            return reinterpret_cast<const u8*>(this)[x];
        }

        inline u8 &operator[](size_t x)
        {
            return reinterpret_cast<u8*>(this)[x];
        }
    };

    static_assert(sizeof(Pixel) == 4, "!");
    static_assert(std::is_pod<Pixel>::value, "!");

} }
