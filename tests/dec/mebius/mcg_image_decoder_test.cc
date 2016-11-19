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

#include "dec/mebius/mcg_image_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::mebius;

static void compress(
    const bstr &input, io::BaseByteStream &output_stream, const u8 marker)
{
    output_stream.write<u8>(marker);
    size_t i = 0;
    while (i < input.size())
    {
        size_t repetitions = 1;
        while (i + repetitions < input.size()
            && input[i + repetitions] == input[i]
            && repetitions < 255)
        {
            repetitions++;
        }

        if (repetitions > 1 || input[i] == marker)
        {
            output_stream.write<u8>(marker);
            output_stream.write<u8>(repetitions);
            output_stream.write<u8>(input[i]);
            i += repetitions;
        }
        else
        {
            output_stream.write<u8>(input[i]);
            i++;
        }
    }
}

TEST_CASE("Studio Mebius MCG images", "[dec]")
{
    const auto decoder = McgImageDecoder();

    SECTION("Type 0")
    {
        const auto expected_image = tests::get_opaque_test_image();

        io::File test_file;
        test_file.stream.write("MCG"_b);
        test_file.stream.write<u8>(0);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        for (const auto c : algo::range(3))
        for (const auto y : algo::range(expected_image.height() - 1, -1, -1))
        for (const auto x : algo::range(expected_image.width()))
            test_file.stream.write<u8>(expected_image.at(x, y)[c]);

        const auto actual_image = tests::decode(decoder, test_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Type 1")
    {
        const auto expected_image = tests::get_transparent_test_image();

        io::File test_file;
        test_file.stream.write("MCG"_b);
        test_file.stream.write<u8>(1);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        for (const auto y : algo::range(expected_image.height() - 1, -1, -1))
        for (const auto x : algo::range(expected_image.width()))
        {
            const auto &pixel = expected_image.at(x, y);
            test_file.stream.write<u8>(pixel.b);
            test_file.stream.write<u8>(pixel.g);
            test_file.stream.write<u8>(pixel.r);
            test_file.stream.write<u8>(pixel.a);
        }

        const auto actual_image = tests::decode(decoder, test_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Type 2")
    {
        const auto expected_image = tests::get_opaque_test_image();

        io::File test_file;
        test_file.stream.write("MCG"_b);
        test_file.stream.write<u8>(2);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());

        bstr data_orig;
        for (const auto c : algo::range(3))
        for (const auto y : algo::range(expected_image.height() - 1, -1, -1))
        for (const auto x : algo::range(expected_image.width()))
            data_orig += expected_image.at(x, y)[c];

        compress(data_orig, test_file.stream, 0xFF);

        const auto actual_image = tests::decode(decoder, test_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Type 3")
    {
        const auto expected_image = tests::get_transparent_test_image();

        io::File test_file;
        test_file.stream.write("MCG"_b);
        test_file.stream.write<u8>(3);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());

        bstr data_orig;
        for (const auto c : algo::range(4))
        for (const auto y : algo::range(expected_image.height() - 1, -1, -1))
        for (const auto x : algo::range(expected_image.width()))
            data_orig += expected_image.at(x, y)[c];

        compress(data_orig, test_file.stream, 0xFF);

        const auto actual_image = tests::decode(decoder, test_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Type 4")
    {
        auto expected_image = tests::get_opaque_test_image();
        for (auto &c : expected_image)
            c.g = c.b = c.r;

        io::File test_file;
        test_file.stream.write("MCG"_b);
        test_file.stream.write<u8>(4);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        for (const auto y : algo::range(expected_image.height() - 1, -1, -1))
        for (const auto x : algo::range(expected_image.width()))
            test_file.stream.write<u8>(expected_image.at(x, y).r);

        const auto actual_image = tests::decode(decoder, test_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Type 5")
    {
        auto expected_image = tests::get_opaque_test_image();
        for (auto &c : expected_image)
            c.g = c.b = c.r;

        io::File test_file;
        test_file.stream.write("MCG"_b);
        test_file.stream.write<u8>(5);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());

        bstr data_orig;
        for (const auto y : algo::range(expected_image.height() - 1, -1, -1))
        for (const auto x : algo::range(expected_image.width()))
            data_orig += expected_image.at(x, y).r;

        compress(data_orig, test_file.stream, 0xFF);

        const auto actual_image = tests::decode(decoder, test_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Type 6")
    {
        res::Image expected_image(800, 600);
        for (auto &c : expected_image)
        {
            c.r = c.g = c.b = 0;
            c.a = 0xFF;
        }

        io::File test_file;
        test_file.stream.write("MCG"_b);
        test_file.stream.write<u8>(6);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        const auto actual_image = tests::decode(decoder, test_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Type 7")
    {
        res::Image expected_image(800, 600);
        for (auto &c : expected_image)
        {
            c.r = c.g = c.b = 0;
            c.a = 0xFF;
        }
        io::File test_file;
        test_file.stream.write("MCG"_b);
        test_file.stream.write<u8>(6);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(0);
        test_file.stream.write_be<u16>(expected_image.width());
        test_file.stream.write_be<u16>(expected_image.height());
        const auto actual_image = tests::decode(decoder, test_file);
        tests::compare_images(actual_image, expected_image);
    }
}
