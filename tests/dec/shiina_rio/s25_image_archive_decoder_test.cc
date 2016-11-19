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

#include "dec/shiina_rio/s25_image_archive_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::shiina_rio;

static const std::string dir = "tests/dec/shiina_rio/files/s25/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::string> &expected_paths)
{
    const auto decoder = S25ImageArchiveDecoder();
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    std::vector<std::shared_ptr<io::File>> expected_files;
    for (const auto &path : expected_paths)
        expected_files.push_back(tests::file_from_path(dir + path));
    tests::compare_images(actual_files, expected_files, false);
}

TEST_CASE("Shiina Rio S25 images", "[dec]")
{
    SECTION("Dummy offsets")
    {
        do_test("BLACK-zlib.S25", {"BLACK-out.png"});
    }

    SECTION("Empty images")
    {
        do_test("BLACK2-zlib.S25", {"BLACK-out.png"});
    }

    SECTION("Alpha channel")
    {
        do_test("CURTAIN1-zlib.S25", {"CURTAIN1-out.png"});
    }

    SECTION("Multi images")
    {
        do_test(
            "DISPDATE-zlib.S25",
            {"DISPDATE_000-out.png", "DISPDATE_001-out.png"});
    }

    SECTION("Incremental pixel data")
    {
        do_test("ERRORF-zlib.S25", {"ERRORF-out.png"});
    }
}
