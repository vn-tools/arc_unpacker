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

#include "dec/kid/lnk_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::kid;

static const std::string dir = "tests/dec/kid/files/lnk/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> expected_files,
    const bool input_file_is_compressed = false)
{
    const auto decoder = LnkArchiveDecoder();
    const auto input_file = input_file_is_compressed
        ? tests::zlib_file_from_path(dir + input_path)
        : tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("Kid LNK archives", "[dec]")
{
    SECTION("Plain")
    {
        do_test(
            "uncompressed.lnk",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }

    SECTION("Compressed")
    {
        do_test(
            "compressed.lnk",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }

    SECTION("Encrypted")
    {
        do_test(
            "encrypted.lnk",
            {
                tests::stub_file("audio.wav", bstr(0x2000, '\xFF')),
                tests::stub_file("image.jpg", bstr(0x2000, '\xFF')),
                tests::stub_file("screensaver.scr", bstr(0x2000, '\xFF')),
            },
            true);
    }
}
