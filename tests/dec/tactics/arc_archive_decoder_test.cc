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

#include "dec/tactics/arc_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::tactics;

static const std::string dir = "tests/dec/tactics/files/arc/";

static void do_test(const std::string &input_path, const bstr &key = ""_b)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("123.txt", "123123123123123123123"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };
    auto decoder = ArcArchiveDecoder();
    decoder.key = key;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("TACTICS ARC archives", "[dec]")
{
    SECTION("Version 0, compressed")
    {
        do_test("v0-compressed.arc");
    }

    SECTION("Version 0, uncompressed")
    {
        do_test("v0-uncompressed.arc");
    }

    SECTION("Version 1, uncompressed")
    {
        do_test("v1-uncompressed.arc", "mlnebzqm"_b);
    }

    SECTION("Version 1, compressed")
    {
        do_test("v1-compressed.arc", "mlnebzqm"_b);
    }
}
