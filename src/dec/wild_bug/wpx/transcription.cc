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

#include "dec/wild_bug/wpx/transcription.h"
#include "dec/wild_bug/wpx/common.h"

using namespace au;
using namespace au::dec::wild_bug::wpx;

static u32 read_count(io::BaseBitStream &bit_stream)
{
    auto n = 0;
    while (!bit_stream.read(1))
        n++;
    u32 ret = 1;
    while (n--)
    {
        ret <<= 1;
        ret += bit_stream.read(1);
    }
    return ret - 1;
}

TranscriptionStrategy1::TranscriptionStrategy1(
    const std::array<size_t, 8> &offsets, s8 quant_size)
    : offsets(offsets), quant_size(quant_size)
{
}

TranscriptionSpec TranscriptionStrategy1::get_spec(DecoderContext &context)
{
    TranscriptionSpec spec;
    if (context.bit_stream.read(1))
    {
        if (context.bit_stream.read(1))
        {
            spec.look_behind = context.input_stream.read<u8>() + 1;
            spec.size = 2;
        }
        else
        {
            spec.look_behind = context.input_stream.read_le<u16>() + 1;
            spec.size = 3;
        }
    }
    else
    {
        spec.look_behind = offsets[context.bit_stream.read(3)];
        spec.size = quant_size == 1 ? 2 : 1;
    }
    spec.size += read_count(context.bit_stream);
    return spec;
}

TranscriptionStrategy2::TranscriptionStrategy2(
    const std::array<size_t, 8> &offsets, s8 quant_size)
    : offsets(offsets), quant_size(quant_size)
{
}

TranscriptionSpec TranscriptionStrategy2::get_spec(DecoderContext &context)
{
    TranscriptionSpec spec;
    spec.look_behind = offsets[context.bit_stream.read(3)];
    spec.size = quant_size == 1 ? 2 : 1;
    spec.size += read_count(context.bit_stream);
    return spec;
}

TranscriptionSpec TranscriptionStrategy3::get_spec(DecoderContext &context)
{
    TranscriptionSpec spec;
    if (context.bit_stream.read(1))
    {
        spec.look_behind = context.input_stream.read<u8>() + 1;
        spec.size = 2;
    }
    else
    {
        spec.look_behind = context.input_stream.read_le<u16>() + 1;
        spec.size = 3;
    }
    spec.size += read_count(context.bit_stream);
    return spec;
}
