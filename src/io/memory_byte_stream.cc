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

#include "io/memory_byte_stream.h"
#include <cstring>
#include "err.h"

using namespace au;
using namespace au::io;

MemoryByteStream::MemoryByteStream(const std::shared_ptr<bstr> input)
    : buffer(input), buffer_pos(0)
{
}

MemoryByteStream::MemoryByteStream()
    : MemoryByteStream(std::make_shared<bstr>(""_b))
{
}

MemoryByteStream::MemoryByteStream(const bstr &buffer)
    : MemoryByteStream(std::make_shared<bstr>(buffer))
{
}

MemoryByteStream::MemoryByteStream(const char *buffer, const size_t buffer_size)
    : MemoryByteStream(std::make_shared<bstr>(buffer, buffer_size))
{
}

MemoryByteStream::MemoryByteStream(io::BaseByteStream &other, const size_t size)
    : MemoryByteStream(std::make_shared<bstr>(other.read(size)))
{
}

MemoryByteStream::MemoryByteStream(io::BaseByteStream &other)
    : MemoryByteStream(std::make_shared<bstr>(other.read_to_eof()))
{
}

MemoryByteStream::~MemoryByteStream()
{
}

io::BaseByteStream &MemoryByteStream::reserve(const uoff_t size)
{
    if (buffer->size() < size)
        buffer->resize(size);
    return *this;
}

void MemoryByteStream::seek_impl(const uoff_t offset)
{
    if (offset > buffer->size())
        throw err::EofError();
    buffer_pos = offset;
}

void MemoryByteStream::read_impl(void *destination, const size_t size)
{
    // destination MUST exist and size MUST be at least 1
    if (buffer_pos + size > buffer->size())
        throw err::EofError();
    auto source_ptr = buffer->get<const u8>() + buffer_pos;
    auto destination_ptr = reinterpret_cast<u8*>(destination);
    buffer_pos += size;
    std::memcpy(destination_ptr, source_ptr, size);
}

void MemoryByteStream::write_impl(const void *source, size_t size)
{
    // source MUST exist and size MUST be at least 1
    reserve(buffer_pos + size);
    auto source_ptr = reinterpret_cast<const u8*>(source);
    auto destination_ptr = buffer->get<u8>() + buffer_pos;
    buffer_pos += size;
    std::memcpy(destination_ptr, source_ptr, size);
}

uoff_t MemoryByteStream::pos() const
{
    return buffer_pos;
}

uoff_t MemoryByteStream::size() const
{
    return buffer->size();
}

void MemoryByteStream::resize_impl(const uoff_t new_size)
{
    buffer->resize(new_size);
    if (buffer_pos > new_size)
        buffer_pos = new_size;
}

std::unique_ptr<io::BaseByteStream> MemoryByteStream::clone() const
{
    auto ret = std::unique_ptr<MemoryByteStream>(new MemoryByteStream(buffer));
    ret->seek(pos());
    return std::move(ret);
}
