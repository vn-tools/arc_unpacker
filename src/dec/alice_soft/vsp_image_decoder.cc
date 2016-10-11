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

#include "dec/alice_soft/vsp_image_decoder.h"
#include "algo/range.h"
#include "dec/alice_soft/pms_image_decoder.h"

using namespace au;
using namespace au::dec::alice_soft;

bool VspImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("vsp");
}

static bstr decompress_vsp(
    io::BaseByteStream &input_stream, const size_t width, const size_t height)
{
    if (width % 8 != 0)
        throw std::logic_error("Invalid width");

    bstr output(width * height);

    auto buf1 = std::make_unique<u8[][4]>(height);
    auto buf2 = std::make_unique<u8[][4]>(height);

    auto bp = buf1.get();
    auto bc = buf2.get();
    auto mask = 0;

    for (const auto x : algo::range(width / 8))
    {
        for (const auto plane : algo::range(4))
        {
            size_t y = 0;
            while (y < height)
            {
                auto c = input_stream.read<u8>();
                switch (c)
                {
                    case 0x00:
                    {
                        auto repetitions = input_stream.read<u8>() + 1;
                        while (repetitions-- && y < height)
                        {
                            bc[y][plane] = bp[y][plane];
                            y++;
                        }
                        break;
                    }

                    case 0x01:
                    {
                        auto repetitions = input_stream.read<u8>() + 1;
                        auto b0 = input_stream.read<u8>();
                        while (repetitions-- && y < height)
                            bc[y++][plane] = b0;
                        break;
                    }

                    case 0x02:
                    {
                        auto repetitions = input_stream.read<u8>() + 1;
                        auto b0 = input_stream.read<u8>();
                        auto b1 = input_stream.read<u8>();
                        while (repetitions-- && y < height)
                        {
                            bc[y++][plane] = b0;
                            if (y >= height) break;
                            bc[y++][plane] = b1;
                        }
                        break;
                    }

                    case 0x03:
                    {
                        auto repetitions = input_stream.read<u8>() + 1;
                        while (repetitions-- && y < height)
                        {
                            bc[y][plane] = bc[y][0] ^ mask;
                            y++;
                        }
                        mask = 0;
                        break;
                    }

                    case 0x04:
                    {
                        auto repetitions = input_stream.read<u8>() + 1;
                        while (repetitions-- && y < height)
                        {
                            bc[y][plane] = bc[y][1] ^ mask;
                            y++;
                        }
                        mask = 0;
                        break;
                    }

                    case 0x05:
                    {
                        auto repetitions = input_stream.read<u8>() + 1;
                        while (repetitions-- && y < height)
                        {
                            bc[y][plane] = bc[y][2] ^ mask;
                            y++;
                        }
                        mask = 0;
                        break;
                    }

                    case 0x06:
                        mask = 0xFF;
                        break;

                    case 0x07:
                        bc[y++][plane] = input_stream.read<u8>();
                        break;

                    default:
                        bc[y++][plane] = c;
                        break;
                }
            }
        }

        for (const auto y : algo::range(height))
        {
            int component[4];
            for (const auto i : algo::range(4))
                component[i] = (bc[y][i] << i) | (bc[y][i] >> (8 - i));

            auto output_ptr = &output[(y * (width / 8) + x) * 8];
            for (const auto j : algo::range(8))
            {
                for (const auto i : algo::range(4))
                    *output_ptr |= (component[i] >> (7 - j)) & (1 << i);
                output_ptr++;
            }
        }

        std::swap(bp, bc);
    }

    return output;
}

res::Image VspImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto x = input_file.stream.read_le<u16>();
    const auto y = input_file.stream.read_le<u16>();
    auto width = input_file.stream.read_le<u16>() - x;
    const auto height = input_file.stream.read_le<u16>() - y;
    const auto use_pms = input_file.stream.read<u8>() > 0;

    std::unique_ptr<res::Image> image;

    if (use_pms)
    {
        width = ((width + 7) / 8) * 8;

        input_file.stream.seek(0x20);
        res::Palette palette(256, input_file.stream, res::PixelFormat::RGB888);

        input_file.stream.seek(0x320);
        const auto pixel_data = PmsImageDecoder::decompress_8bit(
            input_file.stream, width, height);
        image.reset(new res::Image(width, height, pixel_data, palette));
    }
    else
    {
        width *= 8;

        input_file.stream.seek(0x0A);
        res::Palette palette(16);
        for (auto &c : palette)
        {
            c.b = (input_file.stream.read<u8>() & 0x0F) * 0x11;
            c.r = (input_file.stream.read<u8>() & 0x0F) * 0x11;
            c.g = (input_file.stream.read<u8>() & 0x0F) * 0x11;
        }

        input_file.stream.seek(0x3A);
        image = std::make_unique<res::Image>(
            width,
            height,
            decompress_vsp(input_file.stream, width, height),
            palette);
    }

    return *image;
}

static auto _ = dec::register_decoder<VspImageDecoder>("alice-soft/vsp");
