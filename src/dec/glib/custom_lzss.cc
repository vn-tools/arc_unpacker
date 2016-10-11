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

#include "dec/glib/custom_lzss.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

// Modified LZSS routines (repetition count is negated)

using namespace au;
using namespace au::dec;

bstr glib::custom_lzss_decompress(const bstr &input, const size_t output_size)
{
    io::MemoryByteStream output_stream(input);
    return glib::custom_lzss_decompress(output_stream, output_size);
}

bstr glib::custom_lzss_decompress(
    io::BaseByteStream &input_stream, const size_t output_size)
{
    const size_t dict_size = 0x1000;
    size_t dict_pos = 0xFEE;
    u8 dict[dict_size] {};

    bstr output(output_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();

    u16 control = 0;
    while (output_ptr < output_end)
    {
        control >>= 1;
        if (!(control & 0x100))
            control = input_stream.read<u8>() | 0xFF00;

        if (control & 1)
        {
            auto byte = input_stream.read<u8>();
            dict[dict_pos++] = *output_ptr++ = byte;
            dict_pos %= dict_size;
            continue;
        }

        u32 tmp1 = input_stream.read<u8>();
        u32 tmp2 = input_stream.read<u8>();

        u32 look_behind_pos = (((tmp2 & 0xF0) << 4) | tmp1) % dict_size;
        u16 repetitions = (~tmp2 & 0xF) + 3;
        while (repetitions-- && output_ptr < output_end)
        {
            dict[dict_pos++] = *output_ptr++ = dict[look_behind_pos++];
            look_behind_pos %= dict_size;
            dict_pos %= dict_size;
        }
    }
    return output;
}
