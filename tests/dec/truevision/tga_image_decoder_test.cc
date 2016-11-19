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
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::truevision;

static const std::string dir = "tests/dec/truevision/files/tga/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = TgaImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Truevision TGA images", "[dec]")
{
    SECTION("8-bit, compressed")
    {
        do_test("compressed-8-bit.tga", "compressed-8-bit-out.png");
    }

    SECTION("16-bit, compressed")
    {
        do_test("compressed-16-bit.tga", "compressed-16-bit-out.png");
    }

    SECTION("24-bit, compressed")
    {
        do_test("compressed-24-bit.tga", "compressed-24-bit-out.png");
    }

    SECTION("32-bit, compressed")
    {
        do_test("compressed-32-bit.tga", "compressed-32-bit-out.png");
    }

    SECTION("8-bit, uncompressed, palette")
    {
        do_test(
            "uncompressed-8-bit-palette.tga",
            "uncompressed-8-bit-palette-out.png");
    }

    SECTION("16-bit, uncompressed")
    {
        do_test("uncompressed-16-bit.tga", "uncompressed-16-bit-out.png");
    }

    SECTION("24-bit, uncompressed")
    {
        do_test("uncompressed-24-bit.tga", "uncompressed-24-bit-out.png");
    }

    SECTION("32-bit, uncompressed")
    {
        do_test("uncompressed-32-bit.tga", "uncompressed-32-bit-out.png");
    }

    SECTION("16-bit, uncompressed, flipped")
    {
        do_test(
            "uncompressed-16-bit-flipped.tga",
            "uncompressed-16-bit-flipped-out.png");
    }
}
