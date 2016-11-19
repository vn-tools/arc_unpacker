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

#include "algo/pack/zlib.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/common.h"

using namespace au;
using namespace au::algo::pack;

TEST_CASE("ZLIB compression", "[algo][pack]")
{
    const bstr input =
        "\x78\xDA\xCB\xC9\x4C\x4B\x55\xC8"
        "\x2C\x56\x48\xCE\x4F\x49\xE5\x02"
        "\x00\x20\xC1\x04\x62"_b;
    const bstr output = "life is code\n"_b;

    SECTION("Inflating ZLIB from bstr")
    {
        tests::compare_binary(zlib_inflate(input), output);
    }

    SECTION("Inflating ZLIB from stream")
    {
        io::MemoryByteStream input_stream(input);
        tests::compare_binary(zlib_inflate(input_stream), output);
        REQUIRE(input_stream.left() == 0);
    }

    SECTION("Deflating ZLIB from bstr")
    {
        tests::compare_binary(zlib_inflate(zlib_deflate(output)), output);
    }

    SECTION("Deflating ZLIB with RawDeflate")
    {
        const auto deflated = zlib_deflate(output, ZlibKind::RawDeflate);
        const auto inflated = zlib_inflate(deflated, ZlibKind::RawDeflate);
        tests::compare_binary(inflated, output);
    }
}
