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

#include "dec/entis/common/bshf_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::entis::common;

namespace
{
    struct BshfBuffer final
    {
        void decode_buffer();

        bstr key;
        size_t key_offset;

        u8 output[32];
        u8 input[32];
        u8 mask_buf[32];
    };
}

void BshfBuffer::decode_buffer()
{
    key_offset %= key.size();
    for (const auto i : algo::range(32))
    {
        output[i] = 0;
        mask_buf[i] = 0;
    }

    auto pos = key_offset++;
    u8 bit = 0;
    for (const auto i : algo::range(256))
    {
        bit += key[pos++];
        pos %= key.size();

        size_t offset = bit >> 3;
        size_t mask = 0x80 >> (bit & 0x07);
        while (mask_buf[offset] == 0xFF)
        {
            bit += 8;
            offset = bit >> 3;
        }
        while (mask_buf[offset] & mask)
        {
            bit++;
            mask >>= 1;
            if (!mask)
            {
                bit += 8;
                offset = bit >> 3;
                mask = 0x80;
            }
        }
        mask_buf[offset] |= mask;

        if ((input[(i >> 3)] & (0x80 >> (i & 0x07))) != 0)
            output[offset] |= mask;
    }
}

struct BshfDecoder::Priv final
{
    bstr pass_bytes;
};

BshfDecoder::BshfDecoder(const bstr &key) : p(new Priv())
{
    int size = key.size();
    bstr pass_bytes(std::max<size_t>(size, 32));
    for (const auto i : algo::range(key.size()))
        pass_bytes[i] = key[i];
    if (key.empty())
        pass_bytes[size++] = ' ';
    if (size < 32)
    {
        pass_bytes[size++] = 0X1B;
        for (int i = size; i < 32; ++i)
            pass_bytes[i] = pass_bytes[i % size] + pass_bytes[i - 1];
    }
    p->pass_bytes = pass_bytes;
}

BshfDecoder::~BshfDecoder()
{
}

void BshfDecoder::reset()
{
}

void BshfDecoder::decode(u8 *output, const size_t output_size)
{
    auto bshf_buffer = std::make_unique<BshfBuffer>();
    bshf_buffer->key = p->pass_bytes;
    bshf_buffer->key_offset = 0;
    size_t buffer_pos = 32;
    size_t decoded = 0;
    while (decoded < output_size)
    {
        if (buffer_pos >= 32)
        {
            for (const auto i : algo::range(32))
            {
                if (!bit_stream->left())
                    return;
                bshf_buffer->input[i] = bit_stream->read(8);
            }
            bshf_buffer->decode_buffer();
            buffer_pos = 0;
        }
        output[decoded++] = bshf_buffer->output[buffer_pos++];
    }
}
