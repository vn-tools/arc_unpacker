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

#include "algo/locale.h"
#include "test_support/catch.h"
#include "test_support/common.h"

using namespace au;

TEST_CASE("Converting text encoding", "[algo]")
{
    // sjis "あいうえおかきくけこさしすせそたちつてと"
    static const bstr sjis =
        "\x82\xA0\x82\xA2\x82\xA4\x82\xA6\x82\xA8"
        "\x82\xA9\x82\xAB\x82\xAD\x82\xAF\x82\xB1"
        "\x82\xB3\x82\xB5\x82\xB7\x82\xB9\x82\xBB"
        "\x82\xBD\x82\xBF\x82\xC2\x82\xC4\x82\xC6"
        "\x00"_b;

    // utf8 "あいうえおかきくけこさしすせそたちつてと"
    static const bstr utf8 =
        "\xE3\x81\x82\xE3\x81\x84\xE3\x81\x86\xE3"
        "\x81\x88\xE3\x81\x8A\xE3\x81\x8B\xE3\x81"
        "\x8D\xE3\x81\x8F\xE3\x81\x91\xE3\x81\x93"
        "\xE3\x81\x95\xE3\x81\x97\xE3\x81\x99\xE3"
        "\x81\x9B\xE3\x81\x9D\xE3\x81\x9F\xE3\x81"
        "\xA1\xE3\x81\xA4\xE3\x81\xA6\xE3\x81\xA8"
        "\x00"_b;

    SECTION("Converting SJIS to UTF8")
    {
        tests::compare_binary(algo::sjis_to_utf8(sjis), utf8);
    }

    SECTION("Converting UTF8 to SJIS")
    {
        tests::compare_binary(algo::utf8_to_sjis(utf8), sjis);
    }
}

TEST_CASE("Normalizing SJIS strings", "[algo]")
{
    const auto wave_dash = "\xE3\x80\x9C"_b;
    const auto fullwidth_tilde = "\xEF\xBD\x9E"_b;
    tests::compare_binary(
        algo::normalize_sjis(wave_dash),
        algo::normalize_sjis(fullwidth_tilde));
}
