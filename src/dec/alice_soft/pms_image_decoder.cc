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

#include "dec/alice_soft/pms_image_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::alice_soft;

static const bstr magic1 = "PM\x01\x00"_b;
static const bstr magic2 = "PM\x02\x00"_b;

bstr PmsImageDecoder::decompress_8bit(
    io::BaseByteStream &input_stream, const size_t width, const size_t height)
{
    bstr output(width * height);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<u8>();

    while (output_ptr < output_end)
    {
        u8 c = input_stream.read<u8>();

        switch (c)
        {
            case 0xFF:
            {
                auto repetitions = input_stream.read<u8>() + 3;
                while (repetitions-- && output_ptr < output_end)
                {
                    *output_ptr = output_ptr[-width];
                    output_ptr++;
                }
                break;
            }

            case 0xFE:
            {
                auto repetitions = input_stream.read<u8>() + 3;
                while (repetitions-- && output_ptr < output_end)
                {
                    *output_ptr = output_ptr[-width * 2];
                    output_ptr++;
                }
                break;
            }

            case 0xFD:
            {
                auto repetitions = input_stream.read<u8>() + 4;
                auto color = input_stream.read<u8>();
                while (repetitions-- && output_ptr < output_end)
                    *output_ptr++ = color;
                break;
            }

            case 0xFC:
            {
                auto repetitions = input_stream.read<u8>() + 3;
                auto color1 = input_stream.read<u8>();
                auto color2 = input_stream.read<u8>();
                while (repetitions-- && output_ptr < output_end)
                {
                    *output_ptr++ = color1;
                    *output_ptr++ = color2;
                }
                break;
            }

            case 0xF8:
                *output_ptr++ = input_stream.read<u8>();
                break;

            default:
                *output_ptr++ = c;
        }
    }

    return output;
}

bstr PmsImageDecoder::decompress_16bit(
    io::BaseByteStream &input_stream, size_t width, size_t height)
{
    bstr output(width * height * 2);
    auto output_ptr = output.get<u16>();
    auto output_end = output.end<const u16>();

    while (output_ptr < output_end)
    {
        u8 c = input_stream.read<u8>();

        switch (c)
        {
            case 0xFF:
            {
                auto repetitions = input_stream.read<u8>() + 2;
                while (repetitions-- && output_ptr < output_end)
                {
                    *output_ptr = output_ptr[-width];
                    output_ptr++;
                }
                break;
            }

            case 0xFE:
            {
                auto repetitions = input_stream.read<u8>() + 2;
                while (repetitions-- && output_ptr < output_end)
                {
                    *output_ptr = output_ptr[-width * 2];
                    output_ptr++;
                }
                break;
            }

            case 0xFD:
            {
                auto repetitions = input_stream.read<u8>() + 3;
                auto color = input_stream.read_le<u16>();
                while (repetitions-- && output_ptr < output_end)
                    *output_ptr++ = color;
                break;
            }

            case 0xFC:
            {
                auto repetitions = input_stream.read<u8>() + 2;
                auto color1 = input_stream.read_le<u16>();
                auto color2 = input_stream.read_le<u16>();
                while (repetitions-- && output_ptr < output_end)
                {
                    *output_ptr++ = color1;
                    *output_ptr++ = color2;
                }
                break;
            }

            case 0xFB:
                *output_ptr = output_ptr[-width - 1];
                output_ptr++;
                break;

            case 0xFA:
                *output_ptr = output_ptr[-width + 1];
                output_ptr++;
                break;

            case 0xF9:
            {
                auto repetitions = input_stream.read<u8>() + 1;
                auto byte1 = input_stream.read<u8>();
                auto half1
                    = ((byte1 & 0b11100000) << 8)
                    | ((byte1 & 0b00011000) << 6)
                    | ((byte1 & 0b00000111) << 2);
                while (repetitions-- && output_ptr < output_end)
                {
                    auto byte2 = input_stream.read<u8>();
                    auto half2
                        = ((byte2 & 0b11000000) << 5)
                        | ((byte2 & 0b00111100) << 3)
                        | ((byte2 & 0b00000011) << 0);
                    *output_ptr++ = half1 | half2;
                }
                break;
            }

            case 0xF8:
                *output_ptr++ = input_stream.read_le<u16>();
                break;

            default:
                *output_ptr++ = c | (input_stream.read<u8>() << 8);
                break;
        }
    }

    return output;
}

bool PmsImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic1.size()) == magic1)
        return true;
    input_file.stream.seek(0);
    return input_file.stream.read(magic2.size()) == magic2;
}

res::Image PmsImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(2);
    const auto version = input_file.stream.read_le<u16>();
    input_file.stream.skip(2);
    const auto depth = input_file.stream.read_le<u16>();
    input_file.stream.skip(4 * 4);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    const auto data_offset = input_file.stream.read_le<u32>();
    const auto extra_data_offset = input_file.stream.read_le<u32>();

    std::unique_ptr<res::Image> image;

    if (depth == 8)
    {
        input_file.stream.seek(data_offset);
        const auto pixel_data
            = decompress_8bit(input_file.stream, width, height);
        input_file.stream.seek(extra_data_offset);
        res::Palette palette(
            256,
            input_file.stream,
            version == 1 ? res::PixelFormat::RGB888 : res::PixelFormat::BGR888);
        image.reset(new res::Image(width, height, pixel_data, palette));
    }
    else if (depth == 16)
    {
        input_file.stream.seek(data_offset);
        const auto pixel_data
            = decompress_16bit(input_file.stream, width, height);
        image.reset(new res::Image(
            width, height, pixel_data, res::PixelFormat::BGR565));
        if (extra_data_offset)
        {
            input_file.stream.seek(extra_data_offset);
            const auto mask_data
                = decompress_8bit(input_file.stream, width, height);
            res::Image mask(width, height, mask_data, res::PixelFormat::Gray8);
            image->apply_mask(mask);
        }
    }
    else
    {
        throw err::UnsupportedBitDepthError(depth);
    }

    return *image;
}

static auto _ = dec::register_decoder<PmsImageDecoder>("alice-soft/pms");
