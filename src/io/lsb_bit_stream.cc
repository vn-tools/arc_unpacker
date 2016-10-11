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

#include "io/lsb_bit_stream.h"

using namespace au;
using namespace au::io;

LsbBitStream::LsbBitStream(const bstr &input) : BaseBitStream(input)
{
}

LsbBitStream::LsbBitStream(io::BaseByteStream &input_stream)
    : BaseBitStream(input_stream)
{
}

u32 LsbBitStream::read(const size_t bits)
{
    while (bits_available < bits)
    {
        const auto tmp = input_stream->read<u8>();
        buffer |= tmp << bits_available;
        bits_available += 8;
    }
    const auto mask = (1ull << bits) - 1;
    const auto value = buffer & mask;
    buffer >>= bits;
    bits_available -= bits;
    position += bits;
    return value;
}
