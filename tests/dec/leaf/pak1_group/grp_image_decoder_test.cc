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

#include "dec/leaf/pak1_group/grp_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::leaf;

static const std::string pak1_dir = "tests/dec/leaf/files/pak1/";
static const std::string grp_dir = "tests/dec/leaf/files/grp/";

static void do_test(
    const std::string &input_path,
    const std::string &palette_path,
    const std::string &mask_path,
    const std::string &expected_path)
{
    const auto decoder = GrpImageDecoder();
    const auto input_file = tests::file_from_path(input_path);
    if (!palette_path.empty())
    {
        VirtualFileSystem::register_file(palette_path, [&]()
            {
                return tests::file_from_path(palette_path);
            });
    }
    if (!mask_path.empty())
    {
        VirtualFileSystem::register_file(mask_path, [&]()
            {
                return tests::file_from_path(mask_path);
            });
    }
    const auto expected_file = tests::file_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Leaf GRP images", "[dec]")
{
    SECTION("Palettes")
    {
        do_test(
            pak1_dir + "leaflogo-out.grp",
            pak1_dir + "leaflogo-out.c16",
            "",
            grp_dir + "leaflogo-out.png");
}

    SECTION("Palettes, variant with extra 0 bytes at beginning")
    {
        do_test(
            pak1_dir + "leaf-out.grp",
            pak1_dir + "leaf-out.c16",
            "",
            grp_dir + "leaf-out.png");
    }

    SECTION("Palettes and masks")
    {
        do_test(
            grp_dir + "ase200.grp",
            grp_dir + "ase200.c16",
            grp_dir + "ase200.msk",
            grp_dir + "ase200-out.png");
    }
}
