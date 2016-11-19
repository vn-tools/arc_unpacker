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

#include "dec/leaf/ar10_group/cz10_image_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::leaf;

static const std::string dir = "tests/dec/leaf/files/cz10/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::string> &expected_paths)
{
    const auto decoder = Cz10ImageArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    std::vector<std::shared_ptr<io::File>> expected_files;
    for (const auto &path : expected_paths)
        expected_files.push_back(tests::file_from_path(dir + path));
    tests::compare_images(actual_files, expected_files, false);
}

TEST_CASE("Leaf CZ10 images", "[dec]")
{
    SECTION("Encoding type 1+2+3, regular channel order")
    {
        do_test("cbg01_0.cz1", {"cbg01_0-out.png"});
    }

    SECTION("Encoding type 0, multi images")
    {
        do_test(
            "chi06_2.cz1",
            {
                "chi06_2_000-out.png",
                "chi06_2_001-out.png",
                "chi06_2_002-out.png",
                "chi06_2_003-out.png",
                "chi06_2_004-out.png",
            });
    }
}
