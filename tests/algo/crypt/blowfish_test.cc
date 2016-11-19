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

#include "algo/crypt/blowfish.h"
#include "test_support/catch.h"
#include "test_support/common.h"

using namespace au;
using namespace au::algo::crypt;

TEST_CASE("Blowfish decoding", "[algo][crypt]")
{
    SECTION("Aligned to block size")
    {
        static const bstr test_string = "12345678"_b;
        static const bstr test_key = "test_key"_b;
        const Blowfish bf(test_key);
        tests::compare_binary(
            bf.decrypt(bf.encrypt(test_string)),
            test_string);
    }

    SECTION("Not aligned to block size")
    {
        static const bstr test_key = "test_key"_b;
        const Blowfish bf(test_key);
        tests::compare_binary(
            bf.decrypt(bf.encrypt("1234"_b)),
            "1234\x00\x00\x00\x00"_b);
    }
}
