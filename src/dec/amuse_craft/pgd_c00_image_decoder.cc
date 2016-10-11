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

#include "dec/amuse_craft/pgd_c00_image_decoder.h"
#include <array>
#include "algo/ptr.h"
#include "algo/range.h"
#include "dec/truevision/tga_image_decoder.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::amuse_craft;

static const bstr magic = "00_C"_b;

static bstr decompress(const bstr &input, const size_t size_orig)
{
    bstr output;
    output.reserve(size_orig);
    io::MemoryByteStream input_stream(input);
    std::array<u8, 0xBB8> dict = {0};
    auto dict_ptr = algo::make_cyclic_ptr(dict.data(), dict.size());
    auto written = -static_cast<int>(dict.size());
    u16 control = 0;
    while (output.size() < size_orig)
    {
        control >>= 1;
        if (!(control & 0x100))
            control = input_stream.read<u8>() | 0xFF00;
        if (control & 1)
        {
            const auto look_behind_pos = input_stream.read_le<u16>();
            const auto repetitions = input_stream.read<u8>();
            auto source_ptr
                = algo::make_cyclic_ptr(dict.data(), dict.size())
                + (written < 0 ? 0 : dict_ptr.pos())
                + look_behind_pos;
            for (const auto i : algo::range(repetitions))
            {
                const auto c = *source_ptr++;
                output += c;
                *dict_ptr++ = c;
            }
            written += repetitions;
        }
        else
        {
            const auto repetitions = input_stream.read<u8>();
            const auto chunk = input_stream.read(repetitions);
            output += chunk;
            for (const auto &c : chunk)
                *dict_ptr++ = c;
            written += chunk.size();
        }
    }
    return output;
}

bool PgdC00ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(24);
    return input_file.stream.read(magic.size()) == magic;
}

res::Image PgdC00ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(24 + magic.size());
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto size_comp = input_file.stream.read_le<u32>();
    auto data = input_file.stream.read(size_comp - 12);
    data = decompress(data, size_orig);
    io::File tmp_file("test.tga", data);
    const auto tga_image_decoder = dec::truevision::TgaImageDecoder();
    return tga_image_decoder.decode(logger, tmp_file);
}

static auto _
    = dec::register_decoder<PgdC00ImageDecoder>("amuse-craft/pgd-c00");
