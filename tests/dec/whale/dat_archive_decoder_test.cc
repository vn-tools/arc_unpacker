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

#include "dec/whale/dat_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::whale;

static const std::string dir = "tests/dec/whale/files/dat/";

static void do_test(
    DatArchiveDecoder &decoder,
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("Whale DAT archives", "[dec]")
{
    SECTION("Plain")
    {
        DatArchiveDecoder decoder;
        decoder.add_file_name("123.txt");
        decoder.add_file_name("abc.txt");
        do_test(
            decoder,
            "plain.dat",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }

    SECTION("Compressed")
    {
        DatArchiveDecoder decoder;
        decoder.add_file_name("123.txt");
        decoder.add_file_name("abc.txt");
        decoder.set_game_title("A Dog Story");
        do_test(
            decoder,
            "compressed.dat",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }

    SECTION("Compressed, unknown names")
    {
        DatArchiveDecoder decoder;
        decoder.set_game_title("A Dog Story");
        do_test(
            decoder,
            "compressed.dat",
            {
                tests::stub_file("0000.txt", "1234567890"_b),
                tests::stub_file("0001.txt", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }

    SECTION("Fully encrypted")
    {
        DatArchiveDecoder decoder;
        decoder.add_file_name("123.txt");
        decoder.add_file_name("abc.txt");
        do_test(
            decoder,
            "encrypted.dat",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }
}
