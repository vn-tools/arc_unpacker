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

#include "dec/fc01/common/mrg_decryptor.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::fc01;
using namespace au::dec::fc01::common;

static u16 get_mask(u16 d)
{
    d--;
    d >>= 8;
    u16 result = 0xFF;
    while (d)
    {
        d >>= 1;
        result = (result << 1) | 1;
    }
    return result;
}

struct MrgDecryptor::Priv final
{
    Priv(const bstr &input);
    Priv(const bstr &input, const size_t size_orig);

    io::MemoryByteStream input_stream;
    size_t size_orig;
};

MrgDecryptor::Priv::Priv(const bstr &input) : input_stream(input)
{
    // the size is encoded in the stream
    input_stream.seek(0x104);
    size_orig = input_stream.read_le<u32>();
    input_stream.seek(0);
    size_orig ^= input_stream.read_le<u32>();
    input_stream.seek(4);
}

MrgDecryptor::Priv::Priv(const bstr &input, const size_t size_orig)
    : input_stream(input), size_orig(size_orig)
{
    // the size is given from the outside
}

MrgDecryptor::MrgDecryptor(const bstr &input) : p(new Priv(input))
{
}

MrgDecryptor::MrgDecryptor(const bstr &input, const size_t size_orig)
    : p(new Priv(input, size_orig))
{
}

MrgDecryptor::~MrgDecryptor()
{
}

bstr MrgDecryptor::decrypt_without_key()
{
    return decrypt_with_key(0);
}

bstr MrgDecryptor::decrypt_with_key(const u8 initial_key)
{
    bstr output(p->size_orig);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();

    u16 arr1[0x101] {};
    u16 arr2[0x101] {};
    std::vector<u8> arr3;

    auto key = initial_key;
    for (const auto i : algo::range(0x100))
    {
        auto byte = p->input_stream.read<u8>();
        if (initial_key)
        {
            byte = (((byte << 1) | (byte >> 7)) ^ key);
            key -= i;
        }

        arr2[i] = byte;
        arr1[i + 1] = arr2[i] + arr1[i];
        for (const auto j : algo::range(arr2[i]))
            arr3.push_back(i);
    }

    u16 quant = arr1[0x100];
    if (!quant)
        throw err::CorruptDataError("Unexpected data");
    auto mask = get_mask(quant);
    auto scale = 0x10000 / quant;
    auto a = p->input_stream.read_be<u32>();
    auto b = 0;
    auto c = 0xFFFFFFFF;
    while (output_ptr < output_end)
    {
        c = ((c >> 8) * scale) >> 8;
        auto v = (a - b) / c;
        if (v > quant)
            throw err::CorruptDataError("Invalid quant");
        v = arr3.at(v);
        *output_ptr++ = v;
        b += arr1[v] * c;
        c *= arr2[v];
        while (!(((c + b) ^ b) & 0xFF000000))
        {
            a <<= 8;
            b <<= 8;
            c <<= 8;
            a |= p->input_stream.read<u8>();
        }
        while (c <= mask)
        {
            c = (~b & mask) << 8;
            a = (a << 8) | p->input_stream.read<u8>();
            b <<= 8;
        }
    }

    return output;
}
