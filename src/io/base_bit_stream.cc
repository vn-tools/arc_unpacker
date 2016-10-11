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

#include "io/base_bit_stream.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::io;

BaseBitStream::~BaseBitStream() {}

BaseBitStream::BaseBitStream(const bstr &input) :
    buffer(0),
    bits_available(0),
    position(0),
    own_stream_holder(new MemoryByteStream(input)),
    input_stream(own_stream_holder.get())
{
}

BaseBitStream::BaseBitStream(io::BaseByteStream &input_stream) :
    buffer(0),
    bits_available(0),
    position(0),
    input_stream(&input_stream)
{
}

BaseStream &BaseBitStream::seek(const uoff_t new_pos)
{
    if (new_pos > size())
        throw err::EofError();
    position = (new_pos / 32) * 32;
    bits_available = 0;
    buffer = 0;
    input_stream->seek(position / 8);
    read(new_pos % 32);
    return *this;
}

BaseStream &BaseBitStream::resize(const uoff_t new_size)
{
    throw err::NotSupportedError("Not implemented");
}

uoff_t BaseBitStream::pos() const
{
    return position;
}

uoff_t BaseBitStream::size() const
{
    return input_stream->size() * 8;
}

// Elias Gamma coding
u32 BaseBitStream::read_gamma(const bool stop_mark)
{
    size_t count = 0;
    while (read(1) != stop_mark)
        ++count;
    u32 value = 1;
    while (count--)
    {
        value <<= 1;
        value |= read(1); // one by one to enforce MSB order
    }
    return value;
}

void BaseBitStream::flush()
{
}

void BaseBitStream::write(const size_t bits, const u32 value)
{
    throw err::NotSupportedError("Not implemented");
}
