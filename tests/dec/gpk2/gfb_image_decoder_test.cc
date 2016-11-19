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

#include "dec/gpk2/gfb_image_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::gpk2;

TEST_CASE("GPK2 GFB images", "[dec]")
{
    const auto decoder = GfbImageDecoder();
    const auto input_image = tests::get_opaque_test_image();

    SECTION("Compressed, 24-bit")
    {
        io::File input_file;
        input_file.stream.write("GFB\x20");
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write_le<u32>(0);
        input_file.stream.write_le<u32>(
            input_image.width() * input_image.height() * 3);
        input_file.stream.write_le<u32>(0x40);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write_le<u32>(input_image.width());
        input_file.stream.write_le<u32>(input_image.height());
        input_file.stream.write("??"_b);
        input_file.stream.write_le<u16>(24);
        while (input_file.stream.pos() < 0x40)
            input_file.stream.write("?"_b);

        io::MemoryByteStream pixel_stream;
        for (const auto y : algo::range(input_image.height() - 1, -1, -1))
        for (const auto x : algo::range(input_image.width()))
        {
            pixel_stream.write<u8>(input_image.at(x, y).b);
            pixel_stream.write<u8>(input_image.at(x, y).g);
            pixel_stream.write<u8>(input_image.at(x, y).r);
        }
        const auto data = algo::pack::lzss_compress(
            pixel_stream.seek(0).read_to_eof());

        input_file.stream.write(data);
        input_file.stream.seek(12).write_le<u32>(data.size());

        const auto expected_image = input_image;
        const auto actual_image = tests::decode(decoder, input_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Uncompressed 24-bit")
    {
        io::File input_file;
        input_file.stream.write("GFB\x20");
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write_le<u32>(0);
        input_file.stream.write_le<u32>(
            input_image.width() * input_image.height() * 3);
        input_file.stream.write_le<u32>(0x40);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write_le<u32>(input_image.width());
        input_file.stream.write_le<u32>(input_image.height());
        input_file.stream.write("??"_b);
        input_file.stream.write_le<u16>(24);
        while (input_file.stream.pos() < 0x40)
            input_file.stream.write("?"_b);

        for (const auto y : algo::range(input_image.height() - 1, -1, -1))
        for (const auto x : algo::range(input_image.width()))
        {
            input_file.stream.write<u8>(input_image.at(x, y).b);
            input_file.stream.write<u8>(input_image.at(x, y).g);
            input_file.stream.write<u8>(input_image.at(x, y).r);
        }

        const auto expected_image = input_image;
        const auto actual_image = tests::decode(decoder, input_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Uncompressed 32-bit, opaque")
    {
        io::File input_file;
        input_file.stream.write("GFB\x20");
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write_le<u32>(0);
        input_file.stream.write_le<u32>(
            input_image.width() * input_image.height() * 4);
        input_file.stream.write_le<u32>(0x40);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write_le<u32>(input_image.width());
        input_file.stream.write_le<u32>(input_image.height());
        input_file.stream.write("??"_b);
        input_file.stream.write_le<u16>(32);
        while (input_file.stream.pos() < 0x40)
            input_file.stream.write("?"_b);

        for (const auto y : algo::range(input_image.height() - 1, -1, -1))
        for (const auto x : algo::range(input_image.width()))
        {
            input_file.stream.write<u8>(input_image.at(x, y).b);
            input_file.stream.write<u8>(input_image.at(x, y).g);
            input_file.stream.write<u8>(input_image.at(x, y).r);
            input_file.stream.write<u8>(0);
        }

        const auto expected_image = input_image;
        const auto actual_image = tests::decode(decoder, input_file);
        tests::compare_images(actual_image, expected_image);
    }

    SECTION("Uncompressed 32-bit, transparent")
    {
        io::File input_file;
        input_file.stream.write("GFB\x20");
        input_file.stream.write("JUNK"_b);
        input_file.stream.write("JUNK"_b);

        input_file.stream.write_le<u32>(0);
        input_file.stream.write_le<u32>(
            input_image.width() * input_image.height() * 4);
        input_file.stream.write_le<u32>(0x40);
        input_file.stream.write("JUNK"_b);
        input_file.stream.write_le<u32>(input_image.width());
        input_file.stream.write_le<u32>(input_image.height());
        input_file.stream.write("??"_b);
        input_file.stream.write_le<u16>(32);
        while (input_file.stream.pos() < 0x40)
            input_file.stream.write("?"_b);

        for (const auto y : algo::range(input_image.height() - 1, -1, -1))
        for (const auto x : algo::range(input_image.width()))
        {
            input_file.stream.write<u8>(input_image.at(x, y).b);
            input_file.stream.write<u8>(input_image.at(x, y).g);
            input_file.stream.write<u8>(input_image.at(x, y).r);
            input_file.stream.write<u8>(x ^ y);
        }

        auto expected_image = input_image;
        for (const auto y : algo::range(input_image.height() - 1, -1, -1))
        for (const auto x : algo::range(input_image.width()))
            expected_image.at(x, y).a = x ^ y;
        const auto actual_image = tests::decode(decoder, input_file);
        tests::compare_images(actual_image, expected_image);
    }
}
