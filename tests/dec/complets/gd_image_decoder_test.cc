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

#include "dec/complets/gd_image_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"
#include "io/msb_bit_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::complets;

static bstr compress_custom(const res::Image &input_image)
{
    const auto width = input_image.width();
    std::vector<res::Pixel> pixels;
    for (const auto y : algo::range(input_image.height()))
    for (const auto x : algo::range(width))
        pixels.push_back(input_image.at(x, y));
    std::vector<bool> known(pixels.size(), false);
    auto pixels_ptr = algo::make_ptr(pixels.data(), pixels.size());

    io::MemoryByteStream output_stream;
    {
        io::MsbBitStream bit_stream(output_stream);
        while (pixels_ptr.left())
        {
            size_t match_size = 0;
            while (pixels_ptr.left() > 1 && known[pixels_ptr.pos()])
            {
                match_size++;
                pixels_ptr++;
            }

            if (match_size < 2)
                bit_stream.write(2, match_size);
            else if (match_size < 6)
            {
                bit_stream.write(2, 2);
                bit_stream.write(2, match_size - 2);
            }
            else
            {
                bit_stream.write(2, 3);
                auto match_size_bits = 0;
                auto match_size_copy = match_size + 2;
                while (match_size_copy >>= 1)
                    match_size_bits++;
                for (const auto j : algo::range(3, match_size_bits))
                    bit_stream.write(1, 1);
                bit_stream.write(1, 0);
                bit_stream.write(match_size_bits, match_size + 2);
            }

            bit_stream.write(8, pixels_ptr->b);
            bit_stream.write(8, pixels_ptr->g);
            bit_stream.write(8, pixels_ptr->r);

            if (pixels_ptr.left() > width + 2
                && (pixels_ptr[width - 2] == *pixels_ptr
                || pixels_ptr[width - 1] == *pixels_ptr
                || pixels_ptr[width] == *pixels_ptr
                || pixels_ptr[width + 1] == *pixels_ptr
                || pixels_ptr[width + 2] == *pixels_ptr))
            {
                bit_stream.write(1, 1);
                auto lookahead_ptr
                    = algo::make_ptr(pixels.data(), pixels.size())
                    + pixels_ptr.pos();
                while (lookahead_ptr.left() > width + 2)
                {
                    if (lookahead_ptr[width] == *pixels_ptr)
                    {
                        bit_stream.write(2, 2);
                        lookahead_ptr += width;
                    }
                    else if (lookahead_ptr[width - 1] == *pixels_ptr)
                    {
                        bit_stream.write(2, 1);
                        lookahead_ptr += width - 1;
                    }
                    else if (lookahead_ptr[width + 1] == *pixels_ptr)
                    {
                        bit_stream.write(2, 3);
                        lookahead_ptr += width + 1;
                    }
                    else if (lookahead_ptr[width - 2] == *pixels_ptr)
                    {
                        bit_stream.write(2, 0);
                        bit_stream.write(1, 1);
                        bit_stream.write(1, 0);
                        lookahead_ptr += width - 2;
                    }
                    else if (lookahead_ptr[width + 2] == *pixels_ptr)
                    {
                        bit_stream.write(2, 0);
                        bit_stream.write(1, 1);
                        bit_stream.write(1, 1);
                        lookahead_ptr += width + 2;
                    }
                    else
                    {
                        break;
                    }
                    known[lookahead_ptr.pos()] = true;
                }
                bit_stream.write(2, 0);
                bit_stream.write(1, 0);
            }
            else
            {
                bit_stream.write(1, 0);
            }

            pixels_ptr++;

            match_size = 0;
            while (pixels_ptr.left() > match_size)
            {
                if (pixels_ptr[-1] != pixels_ptr[match_size])
                    break;
                known[pixels_ptr.pos() + match_size] = true;
                match_size++;
            }
        }
        bit_stream.write(2, 3);
        bit_stream.write(32, 0xFFFFFFFF);
    }
    return output_stream.seek(0).read_to_eof();
}

static void do_test(const bstr &magic, const size_t width, const size_t height)
{
    const auto decoder = GdImageDecoder();
    res::Image expected_image(width, height);
    expected_image.overlay(
        tests::get_opaque_test_image(), res::Image::OverlayKind::OverwriteAll);
    for (auto &c : expected_image)
    {
        c.a = 0xFF;
        if (c.r == 0xFF && c.g == 0xFF && c.b == 0xFF)
            c.b--;
    }

    res::Image input_image = expected_image;
    input_image.flip_vertically();

    io::File input_file;
    input_file.stream.write(magic);
    for (const auto y : algo::range((input_image.height() - 1) / 10))
    for (const auto x : algo::range(input_image.width() / 10))
    {
        input_file.stream.write<u8>(input_image.at(x * 10, y * 10).b);
        input_file.stream.write<u8>(input_image.at(x * 10, y * 10).g);
        input_file.stream.write<u8>(input_image.at(x * 10, y * 10).r);
    }

    SECTION("Uncompressed")
    {
        input_file.stream.write<u8>(0x62);
        input_file.stream.write("?");
        for (const auto y : algo::range(input_image.height()))
        for (const auto x : algo::range(input_image.width()))
        {
            input_file.stream.write<u8>(input_image.at(x, y).b);
            input_file.stream.write<u8>(input_image.at(x, y).g);
            input_file.stream.write<u8>(input_image.at(x, y).r);
        }
        const auto actual_image = tests::decode(decoder, input_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("LZSS compressed")
    {
        input_file.stream.write<u8>(0x6C);
        input_file.stream.write("?");
        io::MemoryByteStream tmp_stream;
        for (const auto y : algo::range(input_image.height()))
        for (const auto x : algo::range(input_image.width()))
        {
            tmp_stream.write<u8>(input_image.at(x, y).b);
            tmp_stream.write<u8>(input_image.at(x, y).g);
            tmp_stream.write<u8>(input_image.at(x, y).r);
        }
        algo::pack::BitwiseLzssSettings settings;
        settings.position_bits = 16;
        settings.size_bits = 4;
        settings.initial_dictionary_pos = 1;
        settings.min_match_size = 3;
        input_file.stream.write(
            algo::pack::lzss_compress(
                tmp_stream.seek(0).read_to_eof(), settings));
        const auto actual_image = tests::decode(decoder, input_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Custom compressed")
    {
        input_file.stream.write<u8>(0x70);
        input_file.stream.write("?");
        input_file.stream.write(compress_custom(input_image));
        const auto actual_image = tests::decode(decoder, input_file);
        tests::compare_images(actual_image, expected_image);
    }
}

TEST_CASE("Complet's GD images", "[dec]")
{
    SECTION("GD2")
    {
        do_test("GD2?"_b, 640, 480);
    }

    SECTION("GD3")
    {
        do_test("GD3?"_b, 800, 600);
    }
}
