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

#include "dec/kid/prt_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kid;

static const std::string cps_dir = "tests/dec/kid/files/cps/";
static const std::string prt_dir = "tests/dec/kid/files/prt/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = PrtImageDecoder();
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_file = tests::file_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("KID PRT images", "[dec]")
{
    SECTION("Plain")
    {
        do_test(prt_dir + "bg01a1.prt", prt_dir + "bg01a1-out.png");
    }

    SECTION("Transparency")
    {
        do_test(cps_dir + "yh04adm.prt", prt_dir + "yh04adm-out.png");
    }

    SECTION("8-bit")
    {
        do_test(prt_dir + "saver_sm.prt", prt_dir + "saver_sm-out.png");
    }
}
