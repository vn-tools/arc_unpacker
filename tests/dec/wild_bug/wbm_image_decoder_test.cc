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

#include "dec/wild_bug/wbm_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::wild_bug;

static const std::string dir = "tests/dec/wild_bug/files/wbm/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = WbmImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Wild Bug WBM images", "[dec]")
{
    SECTION("32-bit")
    {
        do_test("S_M01.WBM", "S_M01-out.png");
    }

    SECTION("24-bit")
    {
        do_test("S_Y08.WBM", "S_Y08-out.png");
    }

    SECTION("8-bit")
    {
        do_test("EF_WIPE_LR.WBM", "EF_WIPE_LR-out.png");
    }

    SECTION("External alpha channel")
    {
        do_test("ZD0211.WBM", "ZD0211-out.png");
    }

    SECTION("Unaligned stride")
    {
        do_test("ZD0710.WBM", "ZD0710-out.png");
    }

    SECTION("Transcription strategy 1")
    {
        do_test("ZD0710.WBM", "ZD0710-out.png");
    }

    SECTION("Transcription strategy 2")
    {
        do_test("S_M01.WBM", "S_M01-out.png");
    }

    SECTION("Retrieval strategy 1")
    {
        do_test("S_M01.WBM", "S_M01-out.png");
    }

    SECTION("Retrieval strategy 2")
    {
        do_test("E_AT06S.WBM", "E_AT06S-out.png");
    }

    SECTION("Retrieval strategy 3")
    {
        do_test("ZD0710.WBM", "ZD0710-out.png");
    }
}
