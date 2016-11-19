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

#include "dec/cri/xtx_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::cri;

static const io::path cps_dir = "tests/dec/cri/files/cps/";
static const io::path xtx_dir = "tests/dec/cri/files/xtx/";

static void do_test(
    const io::path &input_path, const io::path &expected_path)
{
    const auto decoder = XtxImageDecoder();
    const auto input_file = tests::zlib_file_from_path(input_path);
    const auto expected_file = tests::file_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("CRI XTX images", "[dec]")
{
    SECTION("Headerless, format 2")
    {
        do_test(xtx_dir / "unk_1186-zlib.xtx", xtx_dir / "unk_1186-out.png");
    }

    SECTION("Extra header, format 0")
    {
        do_test(xtx_dir / "unk_3398-zlib.xtx", xtx_dir / "unk_3398-out.png");
    }

    SECTION("Extra header, format 1")
    {
        do_test(xtx_dir / "unk_1237-zlib.xtx", xtx_dir / "unk_1237-out.png");
    }

    SECTION("Extra header, format 2")
    {
        do_test(xtx_dir / "unk_4140-zlib.xtx", xtx_dir / "unk_4140-out.png");
    }
}
