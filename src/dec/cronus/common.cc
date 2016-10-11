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

#include "dec/cronus/common.h"
#include "algo/range.h"

using namespace au;

u32 dec::cronus::get_delta_key(const bstr &input)
{
    auto key = 0;
    for (const auto c : input)
        key += c;
    return key;
}

void dec::cronus::delta_decrypt(bstr &input, u32 initial_key)
{
    auto current_key = initial_key;
    const auto key_delta = initial_key % 32;
    for (const auto i : algo::range(input.size()))
    {
        input[i] ^= current_key;
        current_key += key_delta;
    }
}
