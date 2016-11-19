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

#include "dec/majiro/rct_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::majiro;

static const std::string dir = "tests/dec/majiro/files/rct/";

static void do_test(
    const std::string &input_path,
    const std::string &expected_path,
    const bstr &key = ""_b)
{
    RctImageDecoder decoder;
    if (!key.empty())
        decoder.key = key;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Majiro RCT images", "[dec]")
{
    SECTION("Version 0")
    {
        do_test("face_dummy.rct", "face_dummy-out.png");
    }

    SECTION("Version 1")
    {
        do_test("ev04_01c.rct", "ev04_01c-out.png");
    }

    SECTION("Encrypted")
    {
        do_test(
            "emocon_21.rct",
            "emocon_21-out.png",
            "\x82\xD6\x82\xDA\x82\xA9\x82\xE9"_b);
    }

    SECTION("Base images and masks")
    {
        VirtualFileSystem::register_file("st_kaoru_b_02_01_m.rct", []()
            {
                return tests::zlib_file_from_path(
                    dir + "st_kaoru_b_02_01_m-zlib.rct",
                    "st_kaoru_b_02_01_m.rct");
            });
        VirtualFileSystem::register_file("st_kaoru_b_02_01_m_.rc8", []()
            {
                return tests::zlib_file_from_path(
                    dir + "st_kaoru_b_02_01_m_-zlib.rc8",
                    "st_kaoru_b_02_01_m_.rc8");
            });
        const auto input_path = "st_kaoru_b_02_14_m-zlib.rct";
        const auto expected_path = "st_kaoru_b_02_14_m-out.png";
        const RctImageDecoder decoder;
        const auto input_file = tests::zlib_file_from_path(dir + input_path);
        const auto expected_file = tests::file_from_path(dir + expected_path);
        const auto actual_image = tests::decode(decoder, *input_file);
        tests::compare_images(actual_image, *expected_file);
    }
}
