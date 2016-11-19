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

#include "algo/crypt/aes.h"
#include "test_support/catch.h"
#include "test_support/common.h"

using namespace au;
using namespace au::algo::crypt;

TEST_CASE("AES decoding", "[algo][crypt]")
{
    static const auto test_iv =
        "\x00\x01\x02\x03"
        "\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B"
        "\x0C\x0D\x0E\x0F"_b;

    static const auto test_key =
        "\x00\x01\x02\x03"
        "\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B"
        "\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13"
        "\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B"
        "\x1C\x1D\x1E\x1F"_b;

    static const auto test_strings =
    {
        "0123456789abcdef"_b,
        "0123456789"_b,
        "0"_b,
    };

    for (const auto test_string : test_strings)
    {
        const auto enc = aes256_encrypt_cbc(test_string, test_iv, test_key);
        const auto dec = aes256_decrypt_cbc(enc, test_iv, test_key);
        REQUIRE(dec != enc);
        tests::compare_binary(dec, test_string);
    }
}
