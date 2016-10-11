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

#include "dec/fc01/common/util.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::fc01;

u8 common::rol8(const u8 x, size_t n)
{
    n &= 7;
    return (x << n) | (x >> (8 - n));
}

bstr common::fix_stride(
    const bstr &input,
    const size_t width,
    const size_t height,
    const size_t depth)
{
    const auto output_stride = width * (depth >> 3);
    const auto input_stride = ((output_stride + 3) / 4) * 4;
    bstr output(height * output_stride);
    for (const auto y : algo::range(height))
    {
        auto output_ptr = &output[y * output_stride];
        const auto *input_ptr = &input[y * input_stride];
        for (const auto x : algo::range(width * (depth >> 3)))
            *output_ptr++ = *input_ptr++;
    }
    return output;
}
