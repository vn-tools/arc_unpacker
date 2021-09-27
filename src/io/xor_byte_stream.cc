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

#include "io/xor_byte_stream.h"
#include "err.h"

using namespace au;
using namespace au::io;

XORByteStream::XORByteStream(io::BaseByteStream& parent_stream) :
    parent_stream(parent_stream.clone())
{
}

XORByteStream::~XORByteStream()
{
}

void XORByteStream::seek_impl(const uoff_t offset)
{
    parent_stream->seek(offset);
}

void XORByteStream::read_impl(void* destination, const size_t size)
{
    u8 k = key();
    for (size_t i = 0; i < size; i++)
        reinterpret_cast<u8*>(destination)[i] = parent_stream->read<u8>() ^ k--;
}

void XORByteStream::write_impl(const void* source, const size_t size)
{
    u8 k = key();
    for (size_t i = 0; i < size; i++)
        parent_stream->write<u8>(reinterpret_cast<const u8*>(source)[i] ^ k--);
}

uoff_t XORByteStream::pos() const
{
    return parent_stream->pos();
}

uoff_t XORByteStream::size() const
{
    return parent_stream->size();
}

void XORByteStream::resize_impl(const uoff_t new_size)
{
    throw err::NotSupportedError("Not implemented");
}

std::unique_ptr<io::BaseByteStream> XORByteStream::clone() const
{
    auto ret = std::make_unique<XORByteStream>(*parent_stream);
    ret->seek(pos());
    return std::move(ret);
}

u8 XORByteStream::key()
{
    // The key starts at 0xff and decrements for each byte in the file until it wraps around.
    // We can quickly determine the starting value of our key from the position using modulo.
    return static_cast<u8>(0xff - pos() % 0x100);
}
