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

#include "dec/twilight_frontier/tfbm_image_decoder.h"
#include "io/file_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::twilight_frontier;

static const std::string dir = "tests/dec/twilight_frontier/files/tfbm/";

static void do_test(
    const std::string &input_path,
    const std::string &expected_path)
{
    const auto decoder = TfbmImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Twilight Frontier TFBM images", "[dec]")
{
    SECTION("32-bit")
    {
        do_test("climaxCutA0000.png", "climaxCutA0000-out.png");
    }

    SECTION("16-bit")
    {
        do_test("unk-02479-4461dee8.dat", "unk-02479-4461dee8-out.png");
    }

    SECTION("8-bit, missing external palette")
    {
        do_test("spellB0000.bmp", "spellB0000-out.png");
    }

    SECTION("8-bit, external palette")
    {
        const auto palette_path = dir + "palette000.bmp";
        VirtualFileSystem::register_file(
            palette_path,
            [&]()
            {
                return tests::file_from_path(palette_path);
            });

        do_test("spellB0000.bmp", "spellB0000-out2.png");
    }
}
