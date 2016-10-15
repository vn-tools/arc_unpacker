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

#include "algo/crypt/crc32.h"
#include "algo/range.h"

using namespace au;

static u32 reverse(u32 x)
{
    x = ((x & 0x55555555) << 1) | ((x >> 1) & 0x55555555);
    x = ((x & 0x33333333) << 2) | ((x >> 2) & 0x33333333);
    x = ((x & 0x0F0F0F0F) << 4) | ((x >> 4) & 0x0F0F0F0F);
    x = (x << 24) | ((x & 0xFF00) << 8) | ((x >> 8) & 0xFF00) | (x >> 24);
    return x;
}

u32 algo::crypt::crc32(const bstr &input)
{
    u32 crc = 0xFFFFFFFF;
    for (const auto c : input)
    {
        u32 byte = reverse(c);
        for (const auto j : algo::range(8))
        {
            if (static_cast<int>(crc ^ byte) < 0)
                crc = (crc << 1) ^ 0x04C11DB7;
            else
                crc = crc << 1;
            byte <<= 1;
        }
    }
    return reverse(~crc);
}
