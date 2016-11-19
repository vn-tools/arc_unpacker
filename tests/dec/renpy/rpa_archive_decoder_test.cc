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

#include "dec/renpy/rpa_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::renpy;

static const std::string dir = "tests/dec/renpy/files/rpa/";

static void test(const std::string &path)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("another.txt", "abcdefghij"_b),
        tests::stub_file("abc.txt", "123"_b),
    };
    const auto decoder = RpaArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("Ren'py RPA archives", "[dec]")
{
    SECTION("Version 3")
    {
        test("v3.rpa");
    }

    SECTION("Version 2")
    {
        test("v2.rpa");
    }

    SECTION("Data prefixes")
    {
        test("prefixes.rpa");
    }
}
