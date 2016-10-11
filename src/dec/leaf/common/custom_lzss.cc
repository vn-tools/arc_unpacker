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

#include "dec/leaf/common/custom_lzss.h"
#include <array>
#include "algo/ptr.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::leaf;

// Modified LZSS routine
// - the bit shifts proceed in opposite direction
// - input is negated
bstr common::custom_lzss_decompress(const bstr &input, const size_t output_size)
{
    std::array<u8, 0x1000> dict = {0};
    auto dict_ptr = algo::make_cyclic_ptr(dict.data(), dict.size()) + 0xFEE;
    bstr output(output_size);
    auto output_ptr = algo::make_ptr(output);
    auto input_ptr = algo::make_ptr(input);
    u16 control = 0;
    while (output_ptr.left() && input_ptr.left())
    {
        control <<= 1;
        if (!(control & 0x80))
            control = (~*input_ptr++ << 8) | 0xFF;

        if ((control >> 15) & 1)
        {
            if (!input_ptr.left()) break;
            const auto b = ~*input_ptr++;
            *output_ptr++ = b;
            *dict_ptr++ = b;
        }
        else
        {
            if (input_ptr.left() < 2) break;
            const u8 lo = ~*input_ptr++;
            const u8 hi = ~*input_ptr++;
            const auto tmp = (hi << 8) | lo;
            const auto look_behind_pos = tmp >> 4;
            auto repetitions = (tmp & 0xF) + 3;
            auto source_ptr
                = algo::make_cyclic_ptr(dict.data(), dict.size())
                + look_behind_pos;
            while (repetitions-- && output_ptr.left())
            {
                const auto b = *source_ptr++;
                *output_ptr++ = b;
                *dict_ptr++ = b;
            }
        }
    }
    return output;
}
