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
#include "algo/binary.h"
#include "algo/crypt/hmac.h"
#include "algo/crypt/md5.h"
#include "algo/crypt/sha1.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/common.h"

using namespace au;
using namespace au::dec::nscripter;

static void transform_block(
    const bstr &key, size_t block_num, u8 *block, const size_t block_size)
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

    for (const auto i : algo::range(block_size))
    {
        i0++;
        i1 += box[i0];
        std::swap(box[i0], box[i1]);
        block[i] ^= box[(box[i0] + box[i1]) & 0xFF];
    }
}

TEST_CASE("NScripter NSA encryption", "[dec]")
{
    bstr input;
    for (const auto i : algo::range(10000))
        input += "weird things"_b;
    const auto key = "weird key"_b;

    bstr encrypted_input = input;
    for (const auto i : algo::range(0, input.size(), 1024))
    {
        transform_block(
            key,
            i / 1024,
            encrypted_input.get<u8>() + i,
            std::max<size_t>(0, std::min<size_t>(input.size() - i, 1024)));
    }

    const size_t chunk_size = 555;
    io::MemoryByteStream base_stream(encrypted_input);
    dec::nscripter::NsaEncryptedStream encrypted_stream(base_stream, key);

    bstr output;
    while (encrypted_stream.left())
    {
        output += encrypted_stream.read(
            std::min<size_t>(encrypted_stream.left(), chunk_size));
    }

    tests::compare_binary(output, input);
}
