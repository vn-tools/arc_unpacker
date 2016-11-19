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

#include "dec/entis/eri_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::entis;

static const std::string dir = "tests/dec/entis/files/eri/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = EriImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Entis ERI images", "[dec]")
{
    SECTION("Huffman, 32-bit, non-flipped")
    {
        do_test("img_rgba32.eri", "img_rgba32-out.png");
    }

    SECTION("Gamma, 32-bit, flipped")
    {
        do_test("cb10_14.eri", "cb10_14-out.png");
    }

    SECTION("Nemesis, 32-bit, flipped")
    {
        do_test("FRM_0201.eri", "FRM_0201-out.png");
    }

    SECTION("Multi images")
    {
        do_test("FRM_0102.eri", "FRM_0102-out.png");
    }

    SECTION("8-bit, non-paletted")
    {
        do_test("font24.eri", "font24-out.png");
    }
}
