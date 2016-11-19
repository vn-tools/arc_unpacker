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

#include "dec/silky/akb_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::silky;

static const io::path base_dir = "tests/dec/";
static const io::path test_dir = base_dir / "silky/files/akb/";

static void do_test(
    std::unique_ptr<io::File> input_file,
    std::unique_ptr<io::File> expected_file)
{
    const auto decoder = AkbImageDecoder();
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Silky AKB images", "[dec]")
{
    SECTION("24-bit")
    {
        do_test(
            tests::file_from_path(test_dir / "HINT02.AKB"),
            tests::file_from_path(test_dir / "HINT02-out.png"));
    }

    SECTION("32-bit")
    {
        do_test(
            tests::file_from_path(test_dir / "BREATH.AKB"),
            tests::file_from_path(test_dir / "BREATH-out.png"));
    }

    SECTION("AKB+")
    {
        VirtualFileSystem::register_file("homura-base.bmp", []()
            {
                return tests::zlib_file_from_path(
                    test_dir / "homura-base-zlib.akb");
            });
        do_test(
            tests::zlib_file_from_path(test_dir / "homura-overlay-zlib.akb"),
            tests::file_from_path(base_dir / "homura.png"));
    }
}
