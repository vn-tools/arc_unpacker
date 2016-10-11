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

#include "dec/ism/isg_image_decoder.h"
#include <array>
#include "algo/ptr.h"
#include "err.h"

using namespace au;
using namespace au::dec::ism;

static const bstr magic = "ISM IMAGEFILE\x00"_b;

static void unpack_v10(
    io::BaseByteStream &input_stream, algo::ptr<u8> &output_ptr)
{
    u16 control = 0;
    while (output_ptr.left())
    {
        control >>= 1;
        if (!(control & 0x100))
            control = input_stream.read<u8>() | 0xFF00;

        const auto b = input_stream.read<u8>();
        if (control & 1)
        {
            auto repetitions = input_stream.read<u8>() + 2;
            while (repetitions-- && output_ptr.left())
                *output_ptr++ = b;
        }
        else
        {
            *output_ptr++ = b;
        }
    }
}

static void unpack_v21(
    io::BaseByteStream &input_stream, algo::ptr<u8> &output_ptr)
{
    std::array<u8, 0x800> dict = {0};
    auto dict_ptr = algo::make_cyclic_ptr(dict.data(), dict.size()) + 0x7F7;

    u16 control = 0;
    while (output_ptr.left())
    {
        control <<= 1;
        if (!(control & 0x80))
            control = (input_stream.read<u8>() << 8) | 0xFF;

        if ((control >> 15) & 1)
        {
            const auto hi = input_stream.read<u8>();
            const auto lo = input_stream.read<u8>();
            const auto look_behind_pos = (hi & 7) << 8 | lo;
            auto repetitions = (hi >> 3) + 3;
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
        else
        {
            const auto b = input_stream.read<u8>();
            *output_ptr++ = b;
            *dict_ptr++ = b;
        }
    }
}

bool IsgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image IsgImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(16);
    const auto version = input_file.stream.read<u8>();
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto size_orig = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    input_file.stream.skip(2);
    size_t color_count = input_file.stream.read<u8>();
    if (!color_count)
        color_count = 256;

    input_file.stream.seek(0x30);
    res::Palette palette(
        color_count, input_file.stream, res::PixelFormat::BGR888X);

    bstr output(width * height);
    auto output_ptr = algo::make_ptr(output);

    if (version == 0x21)
        unpack_v21(input_file.stream, output_ptr);
    else if (version == 0x10)
        unpack_v10(input_file.stream, output_ptr);
    else
        throw err::UnsupportedVersionError(version);

    auto ret = res::Image(width, height, output, palette);
    ret.flip_vertically();
    return ret;
}

static auto _ = dec::register_decoder<IsgImageDecoder>("ism/isg");
