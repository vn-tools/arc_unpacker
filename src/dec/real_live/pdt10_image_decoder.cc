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

// Part of AVG32 engine, that later evolved into RealLive engine

#include "dec/real_live/pdt10_image_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::real_live;

static const bstr magic = "PDT10\x00\x00\x00"_b;

bool Pdt10ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

static bstr decompress_rgb(
    io::BaseByteStream &input_stream, const size_t width, const size_t height)
{
    const auto channels = 3;
    bstr output(width * height * channels);
    auto output_ptr = algo::make_ptr(output);

    u32 control = 0;
    while (output_ptr.left() && input_stream.left())
    {
        control <<= 1;
        if (!(control & 0b10000000'0'00000000))
            control = input_stream.read<u8>() | 0b11111111'0'00000000;

        if (control & 0b10000000)
        {
            if (output_ptr.left() < channels)
                break;
            for (const auto i : algo::range(channels))
                *output_ptr++ = input_stream.read<u8>();
        }
        else
        {
            const int tmp = input_stream.read_le<u16>();
            const auto repetitions = ((tmp & 0x0F) + 1) * channels;
            const auto look_behind = ((tmp >> 4) + 1) * channels;
            output_ptr.append_self(-look_behind, repetitions);
        }
    }

    return output;
}

static bstr decompress_mask(
    io::BaseByteStream &input_stream, const size_t width, const size_t height)
{
    bstr output(width * height);
    auto output_ptr = algo::make_ptr(output);

    u32 control = 0;
    while (output_ptr.left() && input_stream.left())
    {
        control <<= 1;
        if (!(control & 0b10000000'0'00000000))
            control = input_stream.read<u8>() | 0b11111111'0'00000000;

        if (control & 0b10000000)
        {
            *output_ptr++ = input_stream.read<u8>();
        }
        else
        {
            const auto repetitions = input_stream.read<u8>() + 2;
            const auto look_behind = input_stream.read<u8>() + 1;
            output_ptr.append_self(-look_behind, repetitions);
        }
    }

    return output;
}

res::Image Pdt10ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    input_file.stream.skip(4);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    input_file.stream.skip(8);
    const auto mask_offset = input_file.stream.read_le<u32>();

    io::MemoryByteStream color_stream(
        input_file.stream,
        mask_offset ? mask_offset - 32 : input_file.stream.left());
    const auto color_data = decompress_rgb(color_stream, width, height);

    res::Image output_image(
        width, height, color_data, res::PixelFormat::BGR888);

    if (mask_offset)
    {
        io::MemoryByteStream mask_stream(
            input_file.stream
                .seek(mask_offset)
                .read_to_eof());

        const auto mask_data = decompress_mask(mask_stream, width, height);
        const res::Image mask_image(
            width, height, mask_data, res::PixelFormat::Gray8);
        output_image.apply_mask(mask_image);
    }

    return output_image;
}

static auto _ = dec::register_decoder<Pdt10ImageDecoder>("real-live/pdt10");
