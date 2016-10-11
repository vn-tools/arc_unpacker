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

#include "dec/amuse_craft/pgd_ge_image_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::amuse_craft;

static const bstr magic = "GE\x20\x00"_b;

static bstr decompress(const bstr &input, const size_t size_orig)
{
    bstr output(size_orig);
    auto output_ptr = output.get<u8>();
    const auto output_start = output.get<const u8>();
    const auto output_end = output.end<const u8>();
    io::MemoryByteStream input_stream(input);

    u16 control = 0;
    while (output_ptr < output_end)
    {
        control >>= 1;
        if (!(control & 0x100))
            control = input_stream.read<u8>() | 0xFF00;

        if (control & 1)
        {
            u32 tmp = input_stream.read_le<u16>();
            size_t repetitions = 0;
            size_t look_behind = 0;
            if (tmp & 8)
            {
                repetitions = (tmp & 7) + 4;
                look_behind = tmp >> 4;
            }
            else
            {
                tmp = (tmp << 8) | input_stream.read<u8>();
                repetitions = ((((tmp & 0xFFC) >> 2) + 1) << 2) | (tmp & 3);
                look_behind = tmp >> 12;
            }

            auto src_ptr = output_ptr - look_behind;
            if (src_ptr < output_start)
                throw err::BadDataOffsetError();
            while (output_ptr < output_end && repetitions--)
                *output_ptr++ = *src_ptr++;
        }
        else
        {
            size_t repetitions = input_stream.read<u8>();
            const auto src = input_stream.read(repetitions);
            auto src_ptr = src.get<const u8>();
            while (output_ptr < output_end && repetitions--)
                *output_ptr++ = *src_ptr++;
        }
    }

    return output;
}

static inline u8 clamp(const long v)
{
    if (v > 255)
        return 255;
    if (v < 0)
        return 0;
    return v;
}

static bstr apply_filter_2(
    const bstr &input, const size_t width, const size_t height)
{
    const size_t out_stride = width * 3;
    const size_t block_size = width * height / 4;
    auto plane1 = input.get<const s8>() + 0 * block_size;
    auto plane2 = input.get<const s8>() + 1 * block_size;
    auto plane3 = input.get<const u8>() + 2 * block_size;

    bstr output(height * out_stride);
    auto output_ptr = output.get<u8>();

    const std::initializer_list<size_t> indices = {0, 1, width, width + 1};
    for (const auto y : algo::range(height / 2))
    {
        for (const auto x : algo::range(width / 2))
        {
            long value_b = 226 * plane1[0];
            long value_g = -43 * plane1[0] - 89 * plane2[0];
            long value_r = 179 * plane2[0];

            for (const auto index : indices)
            {
                long base = plane3[index] << 7;
                output_ptr[3 * index + 0] = clamp((base + value_b) >> 7);
                output_ptr[3 * index + 1] = clamp((base + value_g) >> 7);
                output_ptr[3 * index + 2] = clamp((base + value_r) >> 7);
            }

            plane1++;
            plane2++;
            plane3 += 2;
            output_ptr += 6;
        }

        plane3 += width;
        output_ptr += out_stride;
    }

    return output;
}

static bstr apply_delta_filter(
    const bstr &delta_spec,
    bstr &input,
    const size_t width,
    const size_t height,
    const size_t channels)
{
    const auto stride = width * channels;
    if (delta_spec.size() != height)
        throw err::BadDataSizeError();
    if (input.size() < width * height * channels)
        throw err::BadDataSizeError();

    bstr output(input);
    for (const auto y : algo::range(height))
    {
        const auto prev_line = output.get<u8>() + (y - 1) * stride;
        const auto dst_line = output.get<u8>() + y * stride;

        if (delta_spec[y] == 1)
        {
            for (const auto x : algo::range(channels, stride))
                dst_line[x] = dst_line[x - channels] - dst_line[x];
        }
        else if (delta_spec[y] == 2)
        {
            for (const auto x : algo::range(stride))
                dst_line[x] = prev_line[x] - dst_line[x];
        }
        else if (delta_spec[y] == 4)
        {
            for (const auto x : algo::range(channels, stride))
            {
                const auto mean = (prev_line[x] + dst_line[x - channels]) / 2;
                dst_line[x] = mean - dst_line[x];
            }
        }
        else
            throw err::CorruptDataError("Unknown delta spec");
    }
    return output;
}

bool PgdGeImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image PgdGeImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    input_file.stream.skip(8);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    input_file.stream.skip(8);
    const auto filter_type = input_file.stream.read_le<u16>();
    input_file.stream.skip(2);
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto size_comp = input_file.stream.read_le<u32>();
    auto data = input_file.stream.read(size_comp);
    data = decompress(data, size_orig);

    if (filter_type == 2)
    {
        data = apply_filter_2(data, width, height);
        return res::Image(width, height, data, res::PixelFormat::BGR888);
    }

    if (filter_type == 3)
    {
        io::MemoryByteStream filter_stream(data);
        filter_stream.skip(2);
        const auto depth = filter_stream.read_le<u16>();
        const auto channels = depth >> 3;
        const auto width2 = filter_stream.read_le<u16>();
        const auto height2 = filter_stream.read_le<u16>();
        if (width2 != width || height2 != height)
            throw err::BadDataSizeError();
        const auto delta_spec = filter_stream.read(height);
        data = filter_stream.read_to_eof();
        data = apply_delta_filter(delta_spec, data, width, height, channels);

        if (channels == 4)
            return res::Image(width, height, data, res::PixelFormat::BGRA8888);
        if (channels == 3)
            return res::Image(width, height, data, res::PixelFormat::BGR888);
        throw err::UnsupportedBitDepthError(depth);
    }

    throw err::NotSupportedError(
        algo::format("Unknown filter: %d", filter_type));
}

static auto _ = dec::register_decoder<PgdGeImageDecoder>("amuse-craft/pgd-ge");
