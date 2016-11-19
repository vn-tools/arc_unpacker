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

#include "dec/bgi/cbg_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::bgi;

static const std::string dir = "tests/dec/bgi/files/cbg/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = CbgImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("BGI CBG images", "[dec]")
{
    SECTION("Version 1, 8-bit")
    {
        do_test("v1/4", "v1/4-out.png");
    }

    SECTION("Version 1, 24-bit")
    {
        do_test("v1/3", "v1/3-out.png");
    }

    SECTION("Version 1, 32-bit")
    {
        do_test("v1/ti_si_de_a1", "v1/ti_si_de_a1-out.png");
    }

    SECTION("Version 2, 8-bit")
    {
        do_test("v2/mask04r", "v2/mask04r-out.png");
    }

    SECTION("Version 2, 24-bit")
    {
        do_test("v2/l_card000", "v2/l_card000-out.png");
    }

    SECTION("Version 2, 32-bit")
    {
        do_test("v2/ms_wn_base", "v2/ms_wn_base-out.png");
    }
}
