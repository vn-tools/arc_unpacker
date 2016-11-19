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

#include "algo/binary.h"
#include "test_support/catch.h"

using namespace au;

TEST_CASE("Binary utilities", "[algo]")
{
    SECTION("Xor with u8 key")
    {
        REQUIRE(algo::unxor("test"_b, 1) == "udru"_b);
    }

    SECTION("Xor with bstr key")
    {
        REQUIRE(algo::unxor("test"_b, "\x01\x02"_b) == "ugrv"_b);
    }

    SECTION("Xor with empty bstr key")
    {
        REQUIRE_THROWS(algo::unxor("test"_b, ""_b));
    }

    SECTION("Bit rotation")
    {
        REQUIRE(algo::rotl<u16>(1, 0) == 0b00000000'00000001);
        REQUIRE(algo::rotl<u16>(1, 1) == 0b00000000'00000010);
        REQUIRE(algo::rotl<u16>(1, 8) == 0b00000001'00000000);
        REQUIRE(algo::rotl<u8>(1, 7) == 0b10000000);
        REQUIRE(algo::rotl<u8>(1, 8) == 0b00000001);
        REQUIRE(algo::rotl<u8>(1, 9) == 0b00000010);

        REQUIRE(algo::rotr<u16>(1, 0) == 0b00000000'00000001);
        REQUIRE(algo::rotr<u16>(1, 1) == 0b10000000'00000000);
        REQUIRE(algo::rotr<u16>(1, 8) == 0b00000001'00000000);
        REQUIRE(algo::rotr<u8>(1, 7) == 0b00000010);
        REQUIRE(algo::rotr<u8>(1, 8) == 0b00000001);
        REQUIRE(algo::rotr<u8>(1, 9) == 0b10000000);
    }
}
