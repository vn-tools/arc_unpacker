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

#include "dec/alice_soft/pms_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::alice_soft;

static const std::string dir = "tests/dec/alice_soft/files/pms/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = PmsImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Alice Soft PMS images", "[dec]")
{
    SECTION("8-bit")
    {
        do_test("CG40000.pm", "CG40000-out.png");
    }

    SECTION("8-bit, inverted channels")
    {
        do_test("ALCG0016.PMS", "ALCG0016-out.png");
    }

    SECTION("16-bit, opaque")
    {
        do_test("G214.PMS", "G214-out.png");
    }

    SECTION("16-bit, transparent")
    {
        do_test("G006.PMS", "G006-out.png");
    }
}
