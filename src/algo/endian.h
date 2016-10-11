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

#include <algorithm>
#include <stdexcept>
#include "types.h"

namespace au {
namespace algo {

    enum class Endianness : u8
    {
        LittleEndian,
        BigEndian,
    };

    namespace priv {

        template<typename T> inline T swap_bytes(const T val)
        {
            T ret = val;
            char *x = reinterpret_cast<char*>(&ret);
            std::reverse(x, x + sizeof(T));
            return ret;
        }

    }

    constexpr Endianness get_machine_endianness()
    {
        // this is verified in the tests
        return Endianness::LittleEndian;
    }

    template<typename T> inline T from_little_endian(const T value)
    {
        return get_machine_endianness() != Endianness::LittleEndian
            ? priv::swap_bytes<T>(value)
            : value;
    }

    template<typename T> inline T from_big_endian(const T value)
    {
        return get_machine_endianness() != Endianness::BigEndian
            ? priv::swap_bytes<T>(value)
            : value;
    }

    template<typename T> inline T to_little_endian(const T value)
    {
        return from_little_endian(value);
    }

    template<typename T> inline T to_big_endian(const T value)
    {
        return from_big_endian(value);
    }

} }
