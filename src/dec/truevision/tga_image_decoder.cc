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

#include "dec/truevision/tga_image_decoder.h"
#include "algo/range.h"
#include "err.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::truevision;

namespace
{
    enum Flags
    {
        RightToLeft = 0x10,
        TopToBottom = 0x20,
        Interleave2 = 0x40,
        Interleave4 = 0x80,
    };
}

static res::Palette read_palette(
    io::BaseByteStream &input_stream, const size_t size, const size_t depth)
{
    res::PixelFormat format;
    if (depth == 32)
        format = res::PixelFormat::BGRA8888;
    else if (depth == 24)
        format = res::PixelFormat::BGR888;
    else if (depth == 16 || depth == 15)
        format = res::PixelFormat::BGR555X;
    else
        throw err::UnsupportedBitDepthError(depth);

    return res::Palette(size, input_stream, format);
}

static bstr read_compressed_data(
    io::BaseByteStream &input_stream,
    const size_t width,
    const size_t height,
    const size_t channels)
{
    const auto size_orig = width * height * channels;
    bstr output;
    output.reserve(size_orig);
    while (output.size() < size_orig)
    {
        const auto control = input_stream.read<u8>();
        const auto repetitions = (control & 0x7F) + 1;
        const bool use_rle = (control & 0x80) != 0;
        if (use_rle)
        {
            const auto chunk = input_stream.read(channels);
            for (const auto i : algo::range(repetitions))
                output += chunk;
        }
        else
        {
            for (const auto i : algo::range(repetitions))
                output += input_stream.read(channels);
        }
    }
    return output;
}

static bstr read_uncompressed_data(
    io::BaseByteStream &input_stream,
    const size_t width,
    const size_t height,
    const size_t channels)
{
    return input_stream.read(width * height * channels);
}

static res::Image get_image_from_palette(
    const bstr &input,
    const size_t width,
    const size_t height,
    const size_t depth,
    const res::Palette &palette)
{
    io::MsbBitStream bit_stream(input);
    res::Image output(width, height);
    for (const auto y : algo::range(height))
    for (const auto x : algo::range(width))
        output.at(x, y) = palette[bit_stream.read(depth)];
    return output;
}

static res::Image get_image_without_palette(
    const bstr &input,
    const size_t width,
    const size_t height,
    const size_t depth)
{
    res::PixelFormat format;
    if (depth == 8)
        format = res::PixelFormat::Gray8;
    else if (depth == 16)
        format = res::PixelFormat::BGRA5551;
    else if (depth == 24)
        format = res::PixelFormat::BGR888;
    else if (depth == 32)
        format = res::PixelFormat::BGRA8888;
    else
        throw err::UnsupportedBitDepthError(depth);
    return res::Image(width, height, input, format);
}

bool TgaImageDecoder::is_recognized_impl(io::File &input_file) const
{
    // there's no magic in the header. there is *optional* footer that *might*
    // contain the magic, but checking for this causes conflicts with certain
    // archives that contain TGA files at the end (they understandably get
    // mistaken for TGA footer).
    return input_file.path.has_extension("tga");
}

res::Image TgaImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto id_size = input_file.stream.read<u8>();
    const bool use_palette = input_file.stream.read<u8>() == 1;
    const auto data_type = input_file.stream.read<u8>();
    const auto palette_start = input_file.stream.read_le<u16>();
    const auto palette_size = input_file.stream.read_le<u16>() - palette_start;
    const auto palette_depth = input_file.stream.read<u8>();
    input_file.stream.skip(4); // x and y
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    auto depth = input_file.stream.read<u8>();
    if (!depth)
        depth = 32;
    const auto flags = input_file.stream.read<u8>();

    const auto channels = depth / 8;
    const auto flip_horizontally = (flags & Flags::RightToLeft) != 0;
    const auto flip_vertically = (flags & Flags::TopToBottom) == 0;
    const auto compressed = (data_type & 8) != 0;
    const size_t interleave
        = flags & Flags::Interleave2 ? 2
        : flags & Flags::Interleave4 ? 4 : 1;

    input_file.stream.skip(id_size);

    std::unique_ptr<res::Palette> palette;
    if (use_palette)
    {
        palette = std::make_unique<res::Palette>(
            read_palette(input_file.stream, palette_size, palette_depth));
    }

    const auto data = compressed
        ? read_compressed_data(input_file.stream, width, height, channels)
        : read_uncompressed_data(input_file.stream, width, height, channels);

    res::Image image = use_palette
        ? get_image_from_palette(data, width, height, depth, *palette)
        : get_image_without_palette(data, width, height, depth);

    if (flip_vertically)
        image.flip_vertically();
    if (flip_horizontally)
        image.flip_horizontally();
    return image;
}

static auto _ = dec::register_decoder<TgaImageDecoder>("truevision/tga");
