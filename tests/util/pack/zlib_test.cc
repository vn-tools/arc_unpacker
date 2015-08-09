#include "test_support/catch.hpp"
#include "util/pack/zlib.h"
#include "types.h"

using namespace au::util::pack;

TEST_CASE("Inflating works")
{
    const bstr input =
        "\x78\xDA\xCB\xC9\x4C\x4B\x55\xC8"
        "\x2C\x56\x48\xCE\x4F\x49\xE5\x02"
        "\x00\x20\xC1\x04\x62\x0A"_b;

    REQUIRE(zlib_inflate(input) == "life is code\n"_b);
}
