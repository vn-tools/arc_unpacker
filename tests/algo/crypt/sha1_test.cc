#include "algo/crypt/sha1.h"
#include "algo/str.h"
#include "test_support/catch.h"

using namespace au;
using namespace au::algo::crypt;

static void compare_hashes(const bstr &actual, const bstr &expected)
{
    INFO(algo::hex(actual.str()) << " != " << algo::hex(expected.str()));
    REQUIRE(actual == expected);
}

TEST_CASE("SHA1", "[algo][crypt]")
{
    compare_hashes(
        algo::crypt::sha1("test"_b),
        "\xA9\x4A\x8F\xE5"
        "\xCC\xB1\x9B\xA6"
        "\x1C\x4C\x08\x73"
        "\xD3\x91\xE9\x87"
        "\x98\x2F\xBB\xD3"_b);
}
