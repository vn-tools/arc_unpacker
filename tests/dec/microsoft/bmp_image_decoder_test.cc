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

#include "dec/microsoft/bmp_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::microsoft;

static const std::string dir = "tests/dec/microsoft/files/bmp/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = BmpImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Microsoft BMP images", "[dec]")
{
    SECTION("1-bit palette")
    {
        do_test("pal1bg.bmp", "pal1bg-out.png");
    }

    SECTION("2-bit palette")
    {
        do_test("pal2.bmp", "pal2-out.png");
    }

    SECTION("4-bit palette")
    {
        do_test("pal4.bmp", "pal4-out.png");
    }

    SECTION("8-bit palette")
    {
        do_test("pal8.bmp", "pal8-out.png");
    }

    SECTION("Implied 8-bit palette")
    {
        do_test("pal8-0.bmp", "pal8-out.png");
    }

    SECTION("Version 4, 8-bit palette")
    {
        do_test("pal8v4.bmp", "pal8-out.png");
    }

    SECTION("Version 5, 8-bit palette")
    {
        do_test("pal8v5.bmp", "pal8-out.png");
    }

    SECTION("16-bit (555X)")
    {
        do_test("rgb16.bmp", "rgb16-out.png");
    }

    SECTION("16-bit (565)")
    {
        do_test("rgb16-565.bmp", "rgb16-565-out.png");
    }

    SECTION("16-bit (231)")
    {
        do_test("rgb16-231.bmp", "rgb16-231-out.png");
    }

    SECTION("16-bit (4444)")
    {
        do_test("rgba16-4444.bmp", "rgba16-4444-out.png");
    }

    SECTION("24-bit")
    {
        do_test("rgb24.bmp", "rgb24-out.png");
    }

    SECTION("24-bit, fake palette")
    {
        do_test("rgb24pal.bmp", "rgb24-out.png");
    }

    SECTION("32-bit, no alpha")
    {
        do_test("rgb32.bmp", "rgb32-out.png");
    }

    SECTION("32-bit, alpha")
    {
        do_test("rgba32.bmp", "rgba32-out.png");
    }

    SECTION("32-bit, fake alpha")
    {
        do_test("rgb32fakealpha.bmp", "rgb32fakealpha-out.png");
    }

    SECTION("Unaligned strides")
    {
        do_test("pal8w126.bmp", "pal8w126-out.png");
        do_test("pal8w125.bmp", "pal8w125-out.png");
        do_test("pal8w124.bmp", "pal8w124-out.png");
    }

    SECTION("Unflipped hack")
    {
        do_test("pal8topdown.bmp", "pal8-out.png");
    }
}
