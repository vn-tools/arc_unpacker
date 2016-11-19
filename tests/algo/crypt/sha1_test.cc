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

#include "algo/crypt/sha1.h"
#include "test_support/catch.h"
#include "test_support/common.h"

using namespace au;
using namespace au::algo::crypt;

TEST_CASE("SHA1", "[algo][crypt]")
{
    tests::compare_binary(
        algo::crypt::sha1("test"_b),
        "\xA9\x4A\x8F\xE5"
        "\xCC\xB1\x9B\xA6"
        "\x1C\x4C\x08\x73"
        "\xD3\x91\xE9\x87"
        "\x98\x2F\xBB\xD3"_b);
}
