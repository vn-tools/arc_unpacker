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

#include "dec/nscripter/nsa_encrypted_stream.h"
#include <array>
#include <cstring>
#include "algo/binary.h"
#include "algo/crypt/hmac.h"
#include "algo/crypt/md5.h"
#include "algo/crypt/sha1.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::nscripter;

static const auto block_size = 1024;

static void transform_block(const bstr &key, size_t block_num, bstr &block)
{
    bstr bn(8);

    {
        size_t i = 0;
        while (i < bn.size() && block_num)
        {
            bn[i++] = block_num & 0xFF;
            block_num >>= 8;
        }
    }

    const auto md5_hash  = algo::crypt::md5(bn);
    const auto sha1_hash = algo::crypt::sha1(bn);
    const auto hmac_key  = algo::unxor(md5_hash, sha1_hash).substr(0, 16);
    const auto hmac_hash = algo::crypt::hmac(
        key, hmac_key, algo::crypt::HmacKind::Sha512);

    std::array<u8, 256> box;
    for (const auto i : algo::range(256))
        box[i] = i;

    u8 index = 0;
    for (const auto i : algo::range(256))
    {
        index = box[i] + hmac_hash[i % hmac_hash.size()] + index;
        std::swap(box[i], box[index]);
    }

    u8 i0 = 0, i1 = 0;
    for (const auto i : algo::range(300))
    {
        i0++;
        i1 += box[i0];
        std::swap(box[i0], box[i1]);
    }

    for (const auto i : algo::range(block.size()))
    {
        i0++;
        i1 += box[i0];
        std::swap(box[i0], box[i1]);
        block[i] ^= box[(box[i0] + box[i1]) & 0xFF];
    }
}

NsaEncryptedStream::NsaEncryptedStream(
    io::BaseByteStream &parent_stream, const bstr &key)
    : parent_stream(parent_stream.clone()), key(key)
{
}

NsaEncryptedStream::~NsaEncryptedStream()
{
}

void NsaEncryptedStream::seek_impl(const uoff_t offset)
{
    parent_stream->seek(offset);
}

void NsaEncryptedStream::read_impl(void *destination, const size_t size)
{
    if (key.empty())
    {
        const auto chunk = parent_stream->read(size);
        std::memcpy(destination, chunk.get<u8>(), size);
        return;
    }

    const auto orig_pos = parent_stream->pos();
    const auto padded_pos = orig_pos & ~(block_size - 1);
    const auto offset_pad = orig_pos % block_size;
    parent_stream->seek(padded_pos);

    bstr full_buffer;
    while (full_buffer.size() < size + offset_pad)
    {
        const auto block_num = parent_stream->pos() / block_size;
        auto block = parent_stream->read(
            std::min<uoff_t>(parent_stream->left(), block_size));
        transform_block(key, block_num, block);
        full_buffer += block;
    }
    if (full_buffer.size() < size)
        throw err::BadDataSizeError();
    std::memcpy(destination, full_buffer.get<const u8>() + offset_pad, size);
    parent_stream->seek(orig_pos + size);
}

void NsaEncryptedStream::write_impl(const void *source, const size_t size)
{
    throw err::NotSupportedError("Not implemented");
}

uoff_t NsaEncryptedStream::pos() const
{
    return parent_stream->pos();
}

uoff_t NsaEncryptedStream::size() const
{
    return parent_stream->size();
}

void NsaEncryptedStream::resize_impl(const uoff_t new_size)
{
    parent_stream->resize(new_size);
}

std::unique_ptr<io::BaseByteStream> NsaEncryptedStream::clone() const
{
    auto ret = std::make_unique<NsaEncryptedStream>(*parent_stream, key);
    ret->seek(pos());
    return std::move(ret);
}
