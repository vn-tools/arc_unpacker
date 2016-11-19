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

#include "dec/purple_software/pb3_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::purple_software;

static const std::string dir = "tests/dec/purple_software/files/pb3/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = Pb3ImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Purple Software PB3 images", "[dec]")
{
    SECTION("Version 1")
    {
        do_test("dialog_saki.pb3", "dialog_saki-out.png");
    }

    SECTION("Version 2")
    {
        do_test("bg401i1.pb3", "bg401i1-out.png");
    }

    SECTION("Version 3")
    {
        do_test("mask009a.pb3", "mask009a-out.png");
    }

    SECTION("Version 5")
    {
        do_test("st-ma_d150.pb3", "st-ma_d150-out.png");
    }

    SECTION("Version 6")
    {
        do_test("fk1-al_a102.pb3", "fk1-al_a102-out2.png");
    }

    SECTION("Version 6 with base images")
    {
        VirtualFileSystem::register_file("fk1-al_a101", []()
            {
                return tests::file_from_path(dir + "fk1-al_a101.png");
            });
        do_test("fk1-al_a102.pb3", "fk1-al_a102-out1.png");
    }
}
