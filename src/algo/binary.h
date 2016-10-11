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
