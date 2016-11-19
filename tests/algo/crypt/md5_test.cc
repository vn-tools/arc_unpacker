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

#include "algo/crypt/md5.h"
#include "test_support/catch.h"
#include "test_support/common.h"

using namespace au;
using namespace au::algo::crypt;

TEST_CASE("MD5", "[algo][crypt]")
{
    SECTION("Plain MD5")
    {
        tests::compare_binary(
            algo::crypt::md5("test"_b),
            "\x09\x8F\x6B\xCD\x46\x21\xD3\x73"
            "\xCA\xDE\x4E\x83\x26\x27\xB4\xF6"_b);
    }

    SECTION("Custom initialization vector")
    {
        tests::compare_binary(
            algo::crypt::md5("test"_b, {0, 0, 0, 0}),
            "\x7E\x8E\xFD\x2F\x05\x58\x82\x92"
            "\x58\xC8\x1F\xC9\x59\x81\xCF\xFF"_b);
    }
}
