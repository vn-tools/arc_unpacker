// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

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
