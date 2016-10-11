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

#include "dec/cyberworks/dat_image_decoder.h"
#include "dec/png/png_image_decoder.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::cyberworks;

namespace
{
    enum ImageParameter
    {
        Flags        = 0,
        Height       = 3,
        Width        = 4,
        OriginalSize = 5,
        AlphaSize    = 6,
        MapSize      = 7,
    };
}

static std::map<int, u32> read_image_parameters(
    io::BaseByteStream &input_stream, const DatPlugin &plugin)
{
    std::map<int, u32> parameters;
    u8 step[3] = {0};
    for (const auto idx : plugin.img_header_order)
    {
        step[0] = input_stream.read<u8>();
        if (step[0] == plugin.img_delim[2])
            step[0] = 0;
        u8 tmp = input_stream.read<u8>();
        while (tmp != plugin.img_delim[1])
        {
            if (tmp == plugin.img_delim[0])
                step[2]++;
            else if (tmp == plugin.img_delim[2])
                step[1] = 0;
            else
                step[1] = tmp;
            tmp = input_stream.read<u8>();
        }
        const u32 value
            = step[2] * plugin.img_delim[0] * plugin.img_delim[0]
            + step[1] * plugin.img_delim[0]
            + step[0];
        step[1] = 0;
        step[2] = 0;
        parameters[idx] = value;
    }
    return parameters;
}

static std::unique_ptr<res::Image> decode_type_a1(
    io::BaseByteStream &input_stream,
    const std::map<int, u32> &parameters)
{
    const auto width = parameters.at(ImageParameter::Width);
    const auto height = parameters.at(ImageParameter::Height);
    const auto bitmap_size = parameters.at(ImageParameter::OriginalSize);
    const auto stride = (width * 3 + 3) & ~3;

    res::PixelFormat format;
    if (bitmap_size == width * height * 3)
    {
        return std::make_unique<res::Image>(
            width, height, input_stream, res::PixelFormat::BGR888);
    }

    if (bitmap_size == width * height)
    {
        return std::make_unique<res::Image>(
            width, height, input_stream, res::PixelFormat::Gray8);
    }

    if (bitmap_size == stride * height)
    {
        auto image = std::make_unique<res::Image>(width, height);
        for (const auto y : algo::range(height))
        {
            const auto row = res::Image(
                width, 1, input_stream.read(stride), res::PixelFormat::BGR888);
            for (const auto x : algo::range(width))
                image->at(x, y) = row.at(x, 0);
        }
        return image;
    }

    throw err::BadDataSizeError();
}

static std::unique_ptr<res::Image> decode_type_a3(
    io::BaseByteStream &input_stream,
    const std::map<int, u32> &parameters)
{
    const auto width = parameters.at(ImageParameter::Width);
    const auto height = parameters.at(ImageParameter::Height);
    const auto stride = (width * 3 + 3) & ~3;

    auto output = bstr(width * height * 4);

    {
        auto output_ptr = output.get<u8>();
        for (const auto y : algo::range(height))
        {
            int src = 0;
            const auto line = input_stream.read(stride);
            auto line_ptr = line.get<const u8>();
            for (const auto x : algo::range(width))
            {
                output_ptr[3] = line_ptr[0];
                output_ptr += 4;
                line_ptr += 3;
            }
        }
    }

    {
        auto output_ptr = output.get<u8>();
        for (const auto y : algo::range(height))
        {
            const auto line = input_stream.read(stride);
            auto line_ptr = line.get<const u8>();
            for (const auto x : algo::range(width))
            {
                output_ptr[0] = line_ptr[0];
                output_ptr[1] = line_ptr[1];
                output_ptr[2] = line_ptr[2];
                output_ptr += 4;
                line_ptr += 3;
            }
        }
    }

    return std::make_unique<res::Image>(
        width, height, output, res::PixelFormat::BGRA8888);
}

static std::unique_ptr<res::Image> decode_type_a4(
    io::BaseByteStream &input_stream,
    const std::map<int, u32> &parameters)
{
    const auto bits_size = parameters.at(ImageParameter::MapSize);
    const auto alpha_size = parameters.at(ImageParameter::AlphaSize);
    const auto width = parameters.at(ImageParameter::Width);
    const auto height = parameters.at(ImageParameter::Height);

    const auto rgb_map = input_stream.read(bits_size);
    const auto alpha_map = input_stream.read(bits_size);
    const auto alpha = input_stream.read(alpha_size);

    auto output = bstr(width * height * 4);
    auto output_ptr = output.get<u8>();

    auto bit = 1;
    auto bit_idx = 0;
    auto alpha_idx = 0;

    for (const auto i : algo::range(width * height))
    {
        const auto has_rgb = (bit & rgb_map.at(bit_idx)) != 0;
        const auto has_alpha = (bit & alpha_map.at(bit_idx)) != 0;
        if (has_alpha || has_rgb)
        {
            output_ptr[0] = input_stream.read<u8>();
            output_ptr[1] = input_stream.read<u8>();
            output_ptr[2] = input_stream.read<u8>();
            if (has_alpha)
            {
                output_ptr[3] = alpha.at(alpha_idx);
                alpha_idx += 3;
            }
            else
            {
                output_ptr[3] = 0xFF;
            }
        }
        output_ptr += 4;

        if (bit == 0x80)
        {
            bit_idx++;
            bit = 1;
        }
        else
        {
            bit <<= 1;
        }
    }

    return std::make_unique<res::Image>(
        width, height, output, res::PixelFormat::BGRA8888);
}

static std::unique_ptr<res::Image> decode_type_a(
    io::BaseByteStream &input_stream, const DatPlugin &plugin)
{
    const auto parameters = read_image_parameters(input_stream, plugin);
    const auto flags = parameters.at(ImageParameter::Flags);

    std::unique_ptr<res::Image> image;
    if (flags == 0)
    {
        image = decode_type_a1(input_stream, parameters);
    }
    else if ((flags & 6) == 2)
    {
        throw err::NotSupportedError("Type a2 images is not supported");
    }
    else if ((flags & 6) == 6)
    {
        if (parameters.at(ImageParameter::MapSize) == 0)
            image = decode_type_a3(input_stream, parameters);
        else
            image = decode_type_a4(input_stream, parameters);
    }

    if (!image)
        throw err::RecognitionError("Unknown image type");

    if (plugin.flip_img_vertically)
        image->flip_vertically();
    return image;
}

static std::unique_ptr<res::Image> decode_type_c(
    const Logger &logger, io::BaseByteStream &input_stream)
{
    input_stream.seek(5);
    io::File pseudo_file("dummy.png", input_stream.read_to_eof());
    return std::make_unique<res::Image>(
        dec::png::PngImageDecoder().decode(logger, pseudo_file));
}

DatImageDecoder::DatImageDecoder(const DatPlugin &plugin) : plugin(plugin)
{
}

bool DatImageDecoder::is_recognized_impl(io::File &input_file) const
{
    const auto magic = input_file.stream.read<u8>();
    return magic == 'a' || magic == 'c';
}

res::Image DatImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto magic = input_file.stream.read<u8>();

    if (magic == 'a')
    {
        input_file.stream.seek(2);
        return *decode_type_a(input_file.stream, plugin);
    }

    if (magic == 'c')
    {
        input_file.stream.seek(5);
        return *decode_type_c(logger, input_file.stream);
    }

    throw err::RecognitionError("Unknown image type");
}

// do not register this decoder - it's only used internally
