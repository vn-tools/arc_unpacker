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

#include "dec/bgi/common.h"

using namespace au;

u8 dec::bgi::get_and_update_key(u32 &key)
{
    const u32 tmp1 = 0x4E35 * (key & 0xFFFF);
    const u32 tmp2 = 0x4E35 * (key >> 16);
    const u32 tmp = 0x15A * key + tmp2 + (tmp1 >> 16);
    key = (tmp << 16) + (tmp1 & 0xFFFF) + 1;
    return static_cast<u8>(tmp & 0x7FFF);
}
