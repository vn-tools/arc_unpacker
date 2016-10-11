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

#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::io;

MsbBitStream::MsbBitStream(const bstr &input)
    : BaseBitStream(input), dirty(false)
{
}

MsbBitStream::MsbBitStream(io::BaseByteStream &input_stream)
    : BaseBitStream(input_stream), dirty(false)
{
}

MsbBitStream::~MsbBitStream()
{
    flush();
}

void MsbBitStream::flush()
{
    if (dirty)
    {
        input_stream->write<u8>(buffer << (8 - bits_available)).skip(-1);
        dirty = false;
    }
}

u32 MsbBitStream::read(const size_t bits)
{
    while (bits_available < bits)
    {
        const auto tmp = input_stream->read<u8>();
        buffer = (buffer << 8) | tmp;
        bits_available += 8;
    }
    const auto mask = (1ull << bits) - 1;
    bits_available -= bits;
    position += bits;
    return (buffer >> bits_available) & mask;
}

void MsbBitStream::write(const size_t bits, const u32 value)
{
    const auto mask = (1ull << bits) - 1;
    buffer <<= bits;
    buffer |= value & mask;
    bits_available += bits;
    position += bits;
    while (bits_available > 8)
    {
        bits_available -= 8;
        input_stream->write<u8>(buffer >> bits_available);
    }
    dirty = bits_available > 0;
}
