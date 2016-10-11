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

#include "dec/active_soft/edt_image_decoder.h"
#include "algo/range.h"
#include "dec/active_soft/custom_bit_stream.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::active_soft;

static const bstr magic = ".TRUE\x8D\x5D\x8C\xCB\x00"_b;
static const bstr diff_magic = ".EDT_DIFF\x00"_b;

static inline u8 clamp(const u8 input, const u8 min, const u8 max)
{
    return std::max<u8>(std::min<u8>(input, max), min);
}

bool EdtImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image EdtImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 4);
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    input_file.stream.skip(4);
    const auto meta_size = input_file.stream.read_le<u32>();
    const auto data_size = input_file.stream.read_le<u32>();
    const auto raw_size = input_file.stream.read_le<u32>();

    res::Pixel transparent_color = {0, 0, 0, 0xFF};
    std::string base_file_name;
    if (meta_size)
    {
        io::MemoryByteStream meta_stream(input_file.stream.read(meta_size));
        if (meta_stream.read(diff_magic.size()) == diff_magic)
        {
            transparent_color
                = res::read_pixel<res::PixelFormat::BGR888>(meta_stream);
            meta_stream.skip(1);
            base_file_name = meta_stream.read_to_eof().str();
        }
    }

    const size_t channels = 3;
    const size_t stride = width * channels;
    const size_t target_size = height * stride;

    std::vector<std::pair<s8, s8>> shift_table;
    for (const auto y : {-4, -3, -2, -1})
    for (const auto x : {-3, -2, -1, 0, 1, 2, 3})
        shift_table.push_back({x, y});
    for (const auto x : {-4, -3, -2, -1})
        shift_table.push_back({x, 0});

    std::vector<size_t> look_behind_table;
    for (const auto shift : shift_table)
        look_behind_table.push_back(
            -(shift.first + shift.second * width) * channels);

    bstr output;
    output.reserve(target_size);

    CustomBitStream bit_stream(input_file.stream.read(data_size));
    io::MemoryByteStream raw_stream(input_file.stream.read(raw_size));
    output += raw_stream.read(channels);
    while (output.size() < target_size)
    {
        if (!bit_stream.read(1))
        {
            output += raw_stream.read(channels);
            continue;
        }

        if (bit_stream.read(1))
        {
            auto look_behind = channels;
            if (bit_stream.read(1))
            {
                const auto idx = bit_stream.read(2) << 3;
                look_behind = look_behind_table[(0x11'19'17'18 >> idx) & 0xFF];
            }
            if (look_behind > output.size())
                throw err::BadDataOffsetError();
            for (const auto i : algo::range(channels))
            {
                u8 b = clamp(output[output.size() - look_behind], 0x02, 0xFD);
                if (bit_stream.read(1))
                {
                    const int b2 = bit_stream.read(1) + 1;
                    b += bit_stream.read(1) ? b2 : -b2;
                }
                output += static_cast<u8>(b);
            }
        }
        else
        {
            const auto look_behind = look_behind_table[bit_stream.read(5)];
            auto repetitions = bit_stream.read_gamma(0) * channels;
            if (look_behind > output.size())
                throw err::BadDataOffsetError();
            while (repetitions-- && output.size() < target_size)
                output += output[output.size() - look_behind];
        }
    }

    auto image = res::Image(width, height, output, res::PixelFormat::BGR888);
    if (!base_file_name.empty())
        for (auto &c : image)
            if (c == transparent_color)
                c.a = 0;
    return image;
}

static auto _ = dec::register_decoder<EdtImageDecoder>("active-soft/edt");
