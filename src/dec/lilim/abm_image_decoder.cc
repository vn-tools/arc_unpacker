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

#include "dec/lilim/abm_image_decoder.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::lilim;

static const bstr magic = "BM"_b;

static bstr decompress_opaque(const bstr &input, const size_t size_hint)
{
    bstr output;
    output.reserve(size_hint);
    io::MemoryByteStream input_stream(input);

    while (input_stream.left())
    {
        const auto control = input_stream.read<u8>();

        if (control == 0x00)
        {
            auto repetitions = input_stream.read<u8>();
            while (repetitions--)
                output += '\x00';
        }

        else if (control == 0xFF)
        {
            auto repetitions = input_stream.read<u8>();
            while (repetitions--)
                output += input_stream.read<u8>();
        }

        else
            output += input_stream.read<u8>();
    }

    return output;
}

static bstr decompress_alpha(const bstr &input, const size_t size_hint)
{
    bstr output;
    output.reserve(size_hint);
    io::MemoryByteStream input_stream(input);

    size_t current_channel = 0;
    while (input_stream.left())
    {
        const auto control = input_stream.read<u8>();

        if (control == 0x00)
        {
            auto repetitions = input_stream.read<u8>();
            while (repetitions--)
            {
                output += '\x00';
                current_channel++;
                if (current_channel == 3)
                {
                    output += '\x00';
                    current_channel = 0;
                }
            }
        }

        else if (control == 0xFF)
        {
            auto repetitions = input_stream.read<u8>();
            while (repetitions--)
            {
                output += input_stream.read<u8>();
                current_channel++;
                if (current_channel == 3)
                {
                    output += '\xFF';
                    current_channel = 0;
                }
            }
        }

        else
        {
            output += input_stream.read<u8>();
            current_channel++;
            if (current_channel == 3)
            {
                output += control;
                current_channel = 0;
            }
        }
    }

    return output;
}

bool AbmImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("abm")
        && input_file.stream.read(magic.size()) == magic;
}

res::Image AbmImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(18);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    input_file.stream.skip(2);
    const s16 depth = input_file.stream.read_le<u16>();

    const auto size = width * height * 4;
    input_file.stream.seek(54);
    if (depth == -8)
    {
        res::Palette palette(
            256, input_file.stream.read(256 * 4), res::PixelFormat::BGR888X);
        return res::Image(
            width,
            height,
            decompress_opaque(input_file.stream.read_to_eof(), size),
            palette);
    }
    else if (depth == 32)
    {
        return res::Image(
            width,
            height,
            decompress_alpha(input_file.stream.read_to_eof(), size),
            res::PixelFormat::BGRA8888);
    }
    else
        throw err::UnsupportedBitDepthError(depth);
}

static auto _ = dec::register_decoder<AbmImageDecoder>("lilim/abm");
