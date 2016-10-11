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

#include "io/base_byte_stream.h"
#include "algo/endian.h"
#include "algo/range.h"

using namespace au;
using namespace au::io;

BaseByteStream::~BaseByteStream() {}

bstr BaseByteStream::read_to_zero()
{
    bstr output;
    while (left())
    {
        const char c = read<u8>();
        if (c == '\0')
            break;
        output += c;
    }
    return output;
}

bstr BaseByteStream::read_to_zero(const size_t bytes)
{
    const auto output = read(bytes);
    for (const auto i : algo::range(output.size()))
        if (!output[i])
            return output.substr(0, i);
    return output;
}

bstr BaseByteStream::read_to_eof()
{
    return read(size() - pos());
}

bstr BaseByteStream::read_line()
{
    bstr output;
    while (left())
    {
        const auto c = read<u8>();
        if (c == '\0' || c == '\n')
            break;
        if (c != '\r')
            output += c;
    }
    return output;
}

BaseByteStream &BaseByteStream::write(io::BaseByteStream &other_stream)
{
    return write(other_stream, other_stream.left());
}

BaseByteStream &BaseByteStream::write(
    io::BaseByteStream &other_stream, const size_t size)
{
    const auto buffer_size = 16 * 1024;
    size_t left = size;
    for (const auto i : algo::range(0, size, buffer_size))
    {
        const auto bytes_to_transcribe = std::min<size_t>(buffer_size, left);
        write(other_stream.read(bytes_to_transcribe));
        left -= bytes_to_transcribe;
    }
    return *this;
}

BaseByteStream &BaseByteStream::write_zero_padded(
    const bstr &bytes, const size_t target_size)
{
    if (bytes.size() > target_size)
        return write(bytes.substr(0, target_size));
    write(bytes);
    write(bstr(target_size - bytes.size()));
    return *this;
}
