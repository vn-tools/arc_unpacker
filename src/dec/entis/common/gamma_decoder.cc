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

#include "dec/entis/common/gamma_decoder.h"
#include <algorithm>
#include "err.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::entis;
using namespace au::dec::entis::common;

int common::get_gamma_code(io::BaseBitStream &bit_stream)
{
    if (!bit_stream.left())
        return 0;
    if (!bit_stream.read(1))
        return 1;
    auto base = 2;
    auto code = 0;
    while (true)
    {
        if (!bit_stream.left())
            return 0;
        code = (code << 1) | bit_stream.read(1);
        if (!bit_stream.left())
            return 0;
        if (!bit_stream.read(1))
            return code + base;
        base <<= 1;
    }
}

struct GammaDecoder::Priv final
{
    u8 zero_flag;
    size_t available_size;
};

GammaDecoder::GammaDecoder() : p(new Priv())
{
    p->zero_flag = 0;
}

GammaDecoder::~GammaDecoder()
{
}

void GammaDecoder::reset()
{
    if (!bit_stream)
        throw std::logic_error("Trying to reset with unitialized input");

    p->zero_flag = bit_stream->read(1);
    p->available_size = 0;
}

void GammaDecoder::decode(u8 *output, size_t output_size)
{
    if (!bit_stream)
        throw std::logic_error("Trying to decode with unitialized input");

    auto output_ptr = output;
    auto output_end = output + output_size;

    if (!p->available_size)
    {
        p->available_size = get_gamma_code(*bit_stream);
        if (!p->available_size)
            return;
    }

    while (output_ptr < output_end)
    {
        unsigned int size = std::min(p->available_size, output_size);
        p->available_size -= size;
        output_size -= size;

        if (!p->zero_flag)
        {
            while (size--)
                *output_ptr++ = 0;
        }
        else
        {
            while (size--)
            {
                auto sign = bit_stream->read(1) ? 0xFF : 0;
                auto code = get_gamma_code(*bit_stream);
                if (!code)
                    return;
                if (output_ptr >= output_end)
                    throw err::BadDataOffsetError();
                u8 out = (code ^ sign) - sign;
                *output_ptr++ = out;
            }
        }
        if (!output_size)
        {
            if (!p->available_size)
                p->zero_flag ^= 1;
            return;
        }
        p->zero_flag ^= 1;
        p->available_size = get_gamma_code(*bit_stream);
        if (!p->available_size)
            return;
    }
}
