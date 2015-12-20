#include "algo/locale.h"
#include "test_support/catch.h"
#include "types.h"

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
        REQUIRE(algo::sjis_to_utf8(sjis) == utf8);
    }

    SECTION("Converting UTF8 to SJIS")
    {
        REQUIRE(algo::utf8_to_sjis(utf8) == sjis);
    }
}
