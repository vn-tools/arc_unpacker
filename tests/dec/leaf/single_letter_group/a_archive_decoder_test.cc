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

#include "dec/leaf/single_letter_group/a_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::leaf;

static const std::string dir = "tests/dec/leaf/files/a/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> expected_files)
{
    const auto decoder = AArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("Leaf A archives", "[dec]")
{
    SECTION("Plain")
    {
        do_test(
            "plain.a",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }

    SECTION("Compressed")
    {
        do_test(
            "compressed.a",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }

    SECTION("Encrypted (v3)")
    {
        do_test(
            "encrypted-v3.a",
            {
                tests::stub_file(
                    "whatever",
                    "\x03\x00\x00\x00\x01\x00\x00\x00"
                    "\x4A\x55\x4E\x4B\x4A\x55\x4E\x4B"
                    "\x4A\x55\x4E\x4B\x4A\x55\x4E\x4B"
                    "\x4A\x55\x4E\x4B\x4A\x55\x4E\x4B"
                    "\x5F\x60\x61\x00\xC6\xC8\xCA\x00"
                    "\x32\x2B\x36\x00"_b),

                tests::stub_file(
                    "data",
                    "\x07\x00\x00\x00\x01\x00\x00\x00"
                    "\x4A\x55\x4E\x4B\x4A\x55\x4E\x4B"
                    "\x4A\x55\x4E\x4B\x4A\x55\x4E\x4B"
                    "\x4A\x55\x4E\x4B\x4A\x55\x4E\x4B"
                    "\xB6\xB7\xB8\x00\x74\x76\x78\x00"
                    "\x3A\x3D\x40\x00\x08\x0C\x10\x00"
                    "\xDE\xE3\xE8\x00\xBC\xC2\xC8\x00"
                    "\x5F\x66\x2B\x00"_b),
            });
    }
}
