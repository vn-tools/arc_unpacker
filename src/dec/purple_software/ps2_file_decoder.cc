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

#include "dec/purple_software/ps2_file_decoder.h"
#include <array>
#include "algo/binary.h"
#include "algo/ptr.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::purple_software;

static const bstr magic = "PS2A"_b;

static void decrypt(bstr &data, const u32 key, const size_t shift)
{
    for (const auto i : algo::range(data.size()))
        data[i] = algo::rotr<u8>((data[i] - 0x7C) ^ key, shift);
}

static bstr custom_lzss_decompress(const bstr &input, const size_t size_orig)
{
    std::array<u8, 0x800> dict = {0};
    auto dict_ptr = algo::make_cyclic_ptr(dict.data(), dict.size()) + 0x7DF;
    bstr output(size_orig);
    auto output_ptr = algo::make_ptr(output);
    auto input_ptr = algo::make_ptr(input);
    u16 control = 1;
    while (output_ptr.left())
    {
        if (control == 1)
        {
            if (!input_ptr.left()) break;
            control = *input_ptr++ | 0x100;
        }
        if (control & 1)
        {
            if (!input_ptr.left()) break;
            const auto b = *input_ptr++;
            *output_ptr++ = b;
            *dict_ptr++ = b;
        }
        else
        {
            if (input_ptr.left() < 2) break;
            const auto lo = *input_ptr++;
            const auto hi = *input_ptr++;
            const auto look_behind_pos = lo | (hi & 0xE0) << 3;
            auto repetitions = (hi & 0x1F) + 2;
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
        control >>= 1;
    }
    return output;
}

bool Ps2FileDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    const auto size_comp = input_file.stream.seek(0x24).read_le<u32>();
    const auto size_orig = input_file.stream.seek(0x28).read_le<u32>();
    return size_comp != size_orig;
}

std::unique_ptr<io::File> Ps2FileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto tmp = input_file.stream.seek(12).read_le<u32>();
    const auto key = (tmp >> 24) + (tmp >> 3);
    const auto shift = (tmp >> 20) % 5 + 1;

    const auto size_comp = input_file.stream.seek(0x24).read_le<u32>();
    const auto size_orig = input_file.stream.seek(0x28).read_le<u32>();
    auto data = input_file.stream.seek(0x30).read(size_comp);
    decrypt(data, key, shift);

    data = custom_lzss_decompress(data, size_orig);
    data = input_file.stream.seek(0).read(0x30) + data;
    *reinterpret_cast<u32*>(&data[0x24]) = size_orig;

    return std::make_unique<io::File>(input_file.path, data);
}

static auto _ = dec::register_decoder<Ps2FileDecoder>("purple-software/ps2");
