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

#include "dec/team_shanghai_alice/anm_archive_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::team_shanghai_alice;

static const std::string dir = "tests/dec/team_shanghai_alice/files/anm/";

static void do_test(
    const std::string input_path,
    const std::vector<std::string> expected_paths)
{
    const auto decoder = AnmArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    std::vector<std::shared_ptr<io::File>> expected_files;
    for (const auto &path : expected_paths)
        expected_files.push_back(tests::file_from_path(dir + path));
    tests::compare_images(actual_files, expected_files, false);
}

TEST_CASE("Team Shanghai Alice ANM sprite containers", "[dec]")
{
    SECTION("Format 1")
    {
        do_test("face_01_00.anm", {"face_01_00-out.png"});
    }

    SECTION("Format 3")
    {
        do_test("eff01.anm", {"eff01-out.png"});
    }

    SECTION("Format 5")
    {
        do_test("player00.anm", {"player00-out.png"});
    }

    SECTION("Format 7")
    {
        do_test("clouds.anm", {"clouds-out.png"});
    }

    SECTION("Multi images")
    {
        do_test("eff05.anm", {"eff05-out.png", "eff05-out2.png"});
    }
}
