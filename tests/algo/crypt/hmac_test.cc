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

#include "algo/crypt/hmac.h"
#include "test_support/catch.h"
#include "test_support/common.h"

using namespace au;
using namespace au::algo::crypt;

TEST_CASE("HMAC", "[algo][crypt]")
{
    tests::compare_binary(
        algo::crypt::hmac("test"_b, "key"_b, algo::crypt::HmacKind::Sha512),
        "\x28\x7A\x0F\xB8\x9A\x7F\xBD\xFA\x5B\x55\x38\x63\x69\x18\xE5\x37"
        "\xA5\xB8\x30\x65\xE4\xFF\x33\x12\x68\xB7\xAA\xA1\x15\xDD\xE0\x47"
        "\xA9\xB0\xF4\xFB\x5B\x82\x86\x08\xFC\x0B\x63\x27\xF1\x00\x55\xF7"
        "\x63\x7B\x05\x8E\x9E\x0D\xBB\x9E\x69\x89\x01\xA3\xE6\xDD\x46\x1C"_b);
}
