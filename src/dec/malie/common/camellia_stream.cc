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

#include "dec/malie/common/camellia_stream.h"
#include <cstring>
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::malie::common;

CamelliaStream::CamelliaStream(
    io::BaseByteStream &parent_stream, const std::vector<u32> &key)
        : CamelliaStream(parent_stream, key, 0, parent_stream.size())
{
}

CamelliaStream::CamelliaStream(
    io::BaseByteStream &parent_stream,
    const std::vector<u32> &key,
    const uoff_t offset,
    const uoff_t size) :
        key(key),
        parent_stream(parent_stream.clone()),
        parent_stream_offset(offset),
        parent_stream_size(size)
{
    if (key.size())
        camellia = std::make_unique<algo::crypt::Camellia>(key);
}

CamelliaStream::~CamelliaStream()
{
}

void CamelliaStream::seek_impl(const uoff_t offset)
{
    parent_stream->seek(parent_stream_offset + offset);
}

void CamelliaStream::read_impl(void *destination, const size_t size)
{
    if (!camellia)
    {
        const auto chunk = parent_stream->read(size);
        std::memcpy(destination, chunk.get<u8>(), size);
        return;
    }

    const auto old_pos = parent_stream->pos();
    const auto offset_pad = parent_stream->pos() & 0xF;
    const auto offset_start = parent_stream->pos() & ~0xF;
    const auto aligned_size = (offset_pad + size + 0xF) & ~0xF;
    const auto block_count = (aligned_size + 0xF) / 0x10;
    if (block_count == 0)
        return;

    parent_stream->seek(parent_stream_offset
        + ((parent_stream->pos() - parent_stream_offset) - offset_pad));

    io::MemoryByteStream output_stream;
    output_stream.resize(block_count * 16);
    output_stream.seek(0);
    for (const auto i : algo::range(block_count))
    {
        u32 input_block[4];
        u32 output_block[4];
        for (const auto j : algo::range(4))
            input_block[j] = parent_stream->read_le<u32>();
        camellia->decrypt_block_128(
            offset_start + i * 0x10, input_block, output_block);
        for (const auto j : algo::range(4))
            output_stream.write_be<u32>(output_block[j]);
    }
    const auto chunk = output_stream.seek(offset_pad).read(size);
    std::memcpy(destination, chunk.get<u8>(), size);
    parent_stream->seek(old_pos + size);
}

void CamelliaStream::write_impl(const void *source, const size_t size)
{
    throw err::NotSupportedError("Not implemented");
}

uoff_t CamelliaStream::pos() const
{
    return parent_stream->pos() - parent_stream_offset;
}

uoff_t CamelliaStream::size() const
{
    return parent_stream_size;
}

void CamelliaStream::resize_impl(const uoff_t new_size)
{
    parent_stream->resize(new_size);
}

std::unique_ptr<io::BaseByteStream> CamelliaStream::clone() const
{
    auto ret = std::make_unique<CamelliaStream>(
        *parent_stream, key, parent_stream_offset, parent_stream_size);
    ret->seek(pos());
    return std::move(ret);
}
