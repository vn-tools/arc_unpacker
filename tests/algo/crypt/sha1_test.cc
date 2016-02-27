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
