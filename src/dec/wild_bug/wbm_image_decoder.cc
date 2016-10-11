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

#include "dec/wild_bug/wbm_image_decoder.h"
#include <array>
#include "algo/range.h"
#include "dec/wild_bug/wpx/decoder.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::wild_bug;

static const bstr magic = "WPX\x1A""BMP\x00"_b;

static size_t get_stride(size_t width, size_t channels)
{
    return (width * channels + 3) & (~3);
}

static std::array<size_t, 8> get_offsets(size_t channels, size_t stride)
{
    return
    {
        channels,
        channels * 2,
        channels * 3,
        5 * channels >= stride ? channels * 4 : stride - channels,
        5 * channels >= stride ? channels * 5 : stride,
        5 * channels >= stride ? channels * 6 : stride + channels,
        5 * channels >= stride ? channels * 7 : stride + 2 * channels,
        5 * channels >= stride ? channels * 8 : 2 * stride
    };
}

static void remove_pad(
    bstr &data, size_t height, size_t out_stride, size_t in_stride)
{
    for (const auto y : algo::range(height))
    {
        auto output = data.get<u8>() + y * out_stride;
        auto input = data.get<u8>() + y * in_stride;
        auto size = out_stride;
        while (size-- && output < data.end<u8>())
            *output++ = *input++;
    }
}

static res::Image get_image(
    wpx::Decoder &decoder,
    u8 section_id,
    size_t width,
    size_t height,
    size_t channels)
{
    const auto stride = get_stride(width, channels);
    const auto offsets = get_offsets(channels, stride);
    auto data = decoder.read_compressed_section(section_id, channels, offsets);
    remove_pad(data, height, width * channels, stride);
    if (channels == 1)
        return res::Image(width, height, data, res::PixelFormat::Gray8);
    else if (channels == 3)
        return res::Image(width, height, data, res::PixelFormat::BGR888);
    else if (channels == 4)
        return res::Image(width, height, data, res::PixelFormat::BGRA8888);
    else
        throw err::UnsupportedChannelCountError(channels);
}

bool WbmImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image WbmImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    wpx::Decoder decoder(input_file.stream);

    io::MemoryByteStream metadata_stream(decoder.read_plain_section(0x10));
    metadata_stream.skip(4);
    const auto width = metadata_stream.read_le<u16>();
    const auto height = metadata_stream.read_le<u16>();
    metadata_stream.skip(4);
    const auto depth = metadata_stream.read<u8>();

    if (depth != 32 && depth != 24 && depth != 8)
        throw err::UnsupportedBitDepthError(depth);

    auto image = get_image(decoder, 0x11, width, height, depth >> 3);
    if (decoder.has_section(0x13))
    {
        const auto mask = get_image(decoder, 0x13, width, height, 1);
        image.apply_mask(mask);
    }

    return image;
}

static auto _ = dec::register_decoder<WbmImageDecoder>("wild-bug/wbm");
