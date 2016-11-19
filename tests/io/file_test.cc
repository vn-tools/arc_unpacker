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

#include "io/file.h"
#include "test_support/catch.h"

using namespace au;

static void test_guessing_extension(
    const bstr &magic, const std::string &expected_extension)
{
    io::File file("test", magic);
    REQUIRE(!file.path.has_extension(expected_extension));
    file.guess_extension();
    INFO("Extension guess test failed for ." << expected_extension);
    REQUIRE(file.path.has_extension(expected_extension));
}

TEST_CASE("File", "[io]")
{
    SECTION("Virtual files")
    {
        SECTION("Via properties")
        {
            io::File file;
            REQUIRE(file.path == "");
            REQUIRE(file.stream.size() == 0);
        }
        SECTION("Via constructor")
        {
            io::File file("test.path", "test.content"_b);
            REQUIRE(file.path == "test.path");
            REQUIRE(file.stream.read_to_eof() == "test.content"_b);
        }
    }

    SECTION("Physical files")
    {
        io::File file("tests/io/file_test.cc", io::FileMode::Read);
        REQUIRE(file.path == "tests/io/file_test.cc");
        REQUIRE(file.stream.read_to_eof().find("TEST_CASE"_b) != bstr::npos);
    }

    SECTION("Guessing extension")
    {
        test_guessing_extension("abmp"_b, "b");
        test_guessing_extension("IMOAVI"_b, "imoavi");
        test_guessing_extension("\x89PNG"_b, "png");
        test_guessing_extension("BM"_b, "bmp");
        test_guessing_extension("RIFF"_b, "wav");
        test_guessing_extension("OggS"_b, "ogg");
        test_guessing_extension("\xFF\xD8\xFF"_b, "jpeg");
    }
}
