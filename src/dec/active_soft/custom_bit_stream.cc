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

#include "dec/active_soft/custom_bit_stream.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::active_soft;

CustomBitStream::CustomBitStream(const bstr &input) : io::BaseBitStream(input)
{
}

u32 CustomBitStream::read(const size_t bits)
{
    u32 value = 0;
    for (const auto i : algo::range(bits))
    {
        buffer >>= 1;
        if (!bits_available)
        {
            buffer = input_stream->read<u8>();
            bits_available = 8;
        }
        value <<= 1;
        value |= buffer & 1;
        --bits_available;
    }
    position += bits;
    return value;
}
