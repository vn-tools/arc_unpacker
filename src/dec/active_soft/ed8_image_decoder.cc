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

#include "dec/active_soft/ed8_image_decoder.h"
#include "algo/range.h"
#include "dec/active_soft/custom_bit_stream.h"
#include "err.h"

using namespace au;
using namespace au::dec::active_soft;

static const bstr magic = ".8Bit\x8D\x5D\x8C\xCB\x00"_b;

bool Ed8ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image Ed8ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 4);
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    const auto palette_size = input_file.stream.read_le<u32>();
    const auto data_size = input_file.stream.read_le<u32>();
    const res::Palette palette(
        palette_size, input_file.stream, res::PixelFormat::BGR888);

    static const std::vector<std::pair<s8, s8>> shift_table
        {
            {-1,  0}, {+0, -1}, {-2,  0}, {-1, -1},
            {+1, -1}, {+0, -2}, {-2, -1}, {+2, -1},
            {-2, -2}, {-1, -2}, {+1, -2}, {+2, -2},
            {+0, -3}, {-1, -3},
        };

    std::vector<size_t> look_behind_table;
    for (const auto shift : shift_table)
        look_behind_table.push_back(shift.first + shift.second * width);

    bstr output(width * height);
    auto output_ptr = output.get<u8>();
    const auto output_start = output.get<const u8>();
    const auto output_end = output.end<const u8>();

    CustomBitStream bit_stream(input_file.stream.read(data_size));
    while (output_ptr < output_end)
    {
        *output_ptr++ = bit_stream.read(8);
        if (output_ptr >= output_end)
            break;
        if (bit_stream.read(1))
            continue;
        int last_idx = -1;
        while (output_ptr < output_end)
        {
            int idx = 0;
            if (bit_stream.read(1))
            {
                if (bit_stream.read(1))
                    idx = bit_stream.read(1) + 1;
                idx = (idx << 1) + bit_stream.read(1) + 1;
            }
            idx = (idx << 1) + bit_stream.read(1);

            if (idx == last_idx)
                break;
            last_idx = idx;

            int repetitions = bit_stream.read_gamma(0);
            if (idx >= 2)
                repetitions++;

            const auto look_behind = look_behind_table.at(idx);
            if (output_ptr + look_behind < output_start)
                throw err::BadDataOffsetError();
            if (output_ptr + repetitions > output_end)
                throw err::BadDataSizeError();

            while (repetitions-- && output_ptr < output_end)
            {
                *output_ptr = output_ptr[look_behind];
                output_ptr++;
            }
        }
    }

    return res::Image(width, height, output, palette);
}

static auto _ = dec::register_decoder<Ed8ImageDecoder>("active-soft/ed8");
