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

#include "io/file_byte_stream.h"
#include "io/file_system.h"
#include "test_support/catch.h"
#include "test_support/common.h"
#include "test_support/stream_test.h"

using namespace au;

TEST_CASE("FileByteStream", "[io][stream]")
{
    SECTION("Reading from existing files")
    {
        static const bstr png_magic = "\x89PNG"_b;
        io::FileByteStream stream(
            "tests/dec/png/files/reimu_transparent.png", io::FileMode::Read);
        tests::compare_binary(stream.read(png_magic.size()), png_magic);
    }

    SECTION("Creating to files")
    {
        REQUIRE(!io::exists("tests/trash.out"));

        {
            io::FileByteStream stream("tests/trash.out", io::FileMode::Write);
            REQUIRE_NOTHROW(stream.write_le<u32>(1));
        }

        {
            io::FileByteStream stream("tests/trash.out", io::FileMode::Read);
            REQUIRE(stream.read_le<u32>() == 1);
            REQUIRE(stream.size() == 4);
        }

        io::remove("tests/trash.out");
    }

    SECTION("Full test suite")
    {
        tests::stream_test(
            []()
            {
                return std::make_unique<io::FileByteStream>(
                    "tests/trash.out", io::FileMode::Write);
            },
            []() { io::remove("tests/trash.out"); });
    }
}
