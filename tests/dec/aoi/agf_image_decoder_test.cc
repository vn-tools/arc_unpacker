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

#include "dec/aoi/agf_image_decoder.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::aoi;

TEST_CASE("Aoi AGF images", "[dec]")
{
    const auto decoder = AgfImageDecoder();
    const auto input_image = tests::get_transparent_test_image();

    SECTION("Compression strategy 1")
    {
        io::File input_file;
        input_file.stream.write("AGF\x00"_b);
        input_file.stream.write_le<u32>(1);
        input_file.stream.write_le<u32>(0); // file size
        input_file.stream.write_le<u32>(36); // data offset
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write_le<u32>(input_image.width());
        input_file.stream.write_le<u32>(input_image.height());

        for (const auto y : algo::range(input_image.height()))
        {
            input_file.stream.write_le<u32>((input_image.width() << 8) | 1);
            for (const auto x : algo::range(input_image.width()))
            {
                input_file.stream.write<u8>(input_image.at(x, y).b);
                input_file.stream.write<u8>(input_image.at(x, y).g);
                input_file.stream.write<u8>(input_image.at(x, y).r);
                input_file.stream.write<u8>(input_image.at(x, y).a);
            }
        }

        input_file.stream.seek(8).write_le<u32>(input_file.stream.size());
        const auto expected_image = input_image;
        const auto actual_image = tests::decode(decoder, input_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Compression strategy 2")
    {
        io::File input_file;
        input_file.stream.write("AGF\x00"_b);
        input_file.stream.write_le<u32>(1);
        input_file.stream.write_le<u32>(0); // file size
        input_file.stream.write_le<u32>(36); // data offset
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write_le<u32>(input_image.width());
        input_file.stream.write_le<u32>(input_image.height());

        for (const auto y : algo::range(input_image.height()))
        {
            auto x = 0u;
            while (x < input_image.width())
            {
                auto repetitions = 1u;
                const auto pixel = input_image.at(x, y);
                while (x + repetitions < input_image.width()
                    && input_image.at(x + repetitions, y) == pixel)
                {
                    repetitions++;
                }
                input_file.stream.write_le<u32>((repetitions << 8) | 2);
                input_file.stream.write<u8>(pixel.b);
                input_file.stream.write<u8>(pixel.g);
                input_file.stream.write<u8>(pixel.r);
                input_file.stream.write<u8>(pixel.a);
                x += repetitions;
            }
        }

        input_file.stream.seek(8).write_le<u32>(input_file.stream.size());
        const auto expected_image = input_image;
        const auto actual_image = tests::decode(decoder, input_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Compression strategy 3")
    {
        io::File input_file;
        input_file.stream.write("AGF\x00"_b);
        input_file.stream.write_le<u32>(1);
        input_file.stream.write_le<u32>(0); // file size
        input_file.stream.write_le<u32>(36); // data offset
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write_le<u32>(input_image.width());
        input_file.stream.write_le<u32>(input_image.height());

        for (const auto y : algo::range(input_image.height()))
        {
            auto x = 0u;
            while (x < input_image.width())
            {
                auto repetitions = 1u;
                const auto pixel = input_image.at(x, y);
                while (x + repetitions < input_image.width()
                    && input_image.at(x + repetitions, y) == pixel)
                {
                    repetitions++;
                }
                input_file.stream.write_le<u32>(
                    (1 << 16) | (repetitions << 8) | 3);
                input_file.stream.write<u8>(pixel.b);
                input_file.stream.write<u8>(pixel.g);
                input_file.stream.write<u8>(pixel.r);
                input_file.stream.write<u8>(pixel.a);
                x += repetitions;
            }
        }

        input_file.stream.seek(8).write_le<u32>(input_file.stream.size());
        const auto expected_image = input_image;
        const auto actual_image = tests::decode(decoder, input_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Compression strategy 4")
    {
        io::File input_file;
        input_file.stream.write("AGF\x00"_b);
        input_file.stream.write_le<u32>(1);
        input_file.stream.write_le<u32>(0); // file size
        input_file.stream.write_le<u32>(36); // data offset
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write_le<u32>(input_image.width());
        input_file.stream.write_le<u32>(input_image.height());

        for (const auto y : algo::range(input_image.height()))
        {
            auto x = 0u;
            while (x < input_image.width())
            {
                auto repetitions = 1u;
                const auto pixel = input_image.at(x, y);
                while (x + repetitions < input_image.width()
                    && input_image.at(x + repetitions, y) == pixel
                    && repetitions < 4096)
                {
                    repetitions++;
                }
                input_file.stream.write_le<u32>((1 << 8) | 1);
                input_file.stream.write<u8>(pixel.b);
                input_file.stream.write<u8>(pixel.g);
                input_file.stream.write<u8>(pixel.r);
                input_file.stream.write<u8>(pixel.a);
                if (repetitions > 1)
                {
                    input_file.stream.write_le<u32>(
                        ((repetitions - 1) << 20) | (1 << 8) | 4);
                }
                x += repetitions;
            }
        }

        input_file.stream.seek(8).write_le<u32>(input_file.stream.size());
        const auto expected_image = input_image;
        const auto actual_image = tests::decode(decoder, input_file);
        tests::compare_images(actual_image, expected_image);
    }
}
