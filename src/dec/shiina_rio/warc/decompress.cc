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

#include "dec/shiina_rio/warc/decompress.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::shiina_rio;
using namespace au::dec::shiina_rio::warc;

namespace
{
    class CustomBitStream final : public io::BaseBitStream
    {
    public:
        CustomBitStream(const bstr &input);
        u32 read(const size_t bits) override;
    private:
        void fetch();
    };
}

CustomBitStream::CustomBitStream(const bstr &input) : io::BaseBitStream(input)
{
}

void CustomBitStream::fetch()
{
    if (input_stream->left() >= 4)
    {
        buffer = input_stream->read_le<u32>();
        return;
    }
    while (input_stream->left())
    {
        buffer <<= 8;
        buffer |= input_stream->read<u8>();
    }
}

u32 CustomBitStream::read(size_t requested_bits)
{
    u32 value = 0;
    if (bits_available < requested_bits)
    {
        do
        {
            requested_bits -= bits_available;
            const u32 mask = (1ull << bits_available) - 1;
            value |= (buffer & mask) << requested_bits;
            fetch();
            bits_available = 32;
        }
        while (requested_bits > 32);
    }
    bits_available -= requested_bits;
    const u32 mask = (1ull << requested_bits) - 1;
    return value | ((buffer >> bits_available) & mask);
}

static int init_huffman(
    io::BaseBitStream &bit_stream, u16 nodes[2][512], int &size)
{
    if (!bit_stream.read(1))
        return bit_stream.read(8);
    const auto pos = size;
    if (pos > 511)
        return -1;
    size++;
    nodes[0][pos] = init_huffman(bit_stream, nodes, size);
    nodes[1][pos] = init_huffman(bit_stream, nodes, size);
    return pos;
}

static bstr decode_huffman(const bstr &input, const size_t size_orig)
{
    bstr output(size_orig);
    auto output_ptr = output.get<u8>();
    const auto output_end = output.end<const u8>();
    CustomBitStream bit_stream(input);
    u16 nodes[2][512];
    auto size = 256;
    const auto root = init_huffman(bit_stream, nodes, size);
    while (output_ptr < output_end)
    {
        auto byte = root;
        while (byte >= 256 && byte <= 511)
            byte = nodes[bit_stream.read(1)][byte];
        *output_ptr++ = byte;
    }
    return output;
}

bstr warc::decompress_yh1(
    const bstr &input, const size_t size_orig, const bool encrypted)
{
    bstr transient(input);
    if (encrypted)
    {
        const u32 key32 = 0x6393528E;
        const u16 key16 = 0x4B4D;
        for (const auto i : algo::range(transient.size() / 4))
            transient.get<u32>()[i] ^= key32 ^ key16;
    }
    return decode_huffman(transient, size_orig);
}

bstr warc::decompress_ypk(
    const bstr &input, const size_t size_orig, const bool encrypted)
{
    bstr transient(input);
    if (encrypted)
    {
        const u16 key16 = 0x4B4D;
        const u32 key32 = (key16 | (key16 << 16)) ^ 0xFFFFFFFF;
        size_t i = 0;
        while (i < transient.size() / 4)
            transient.get<u32>()[i++] ^= key32;
        i *= 4;
        while (i < transient.size())
            transient[i++] ^= key32;
    }
    return algo::pack::zlib_inflate(transient);
}

bstr warc::decompress_ylz(
    const bstr &input, const size_t size_orig, const bool encrypted)
{
    throw err::NotSupportedError("YLZ decompression not implemented");
}
