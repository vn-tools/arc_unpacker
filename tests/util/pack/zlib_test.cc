#include "io/buffered_io.h"
#include "test_support/catch.hh"
#include "types.h"
#include "util/pack/zlib.h"

using namespace au;
using namespace au::util::pack;

TEST_CASE("Inflating ZLIB streams from bstr", "[util][pack]")
{
    const bstr input =
        "\x78\xDA\xCB\xC9\x4C\x4B\x55\xC8"
        "\x2C\x56\x48\xCE\x4F\x49\xE5\x02"
        "\x00\x20\xC1\x04\x62"_b;

    REQUIRE(zlib_inflate(input) == "life is code\n"_b);
}

TEST_CASE("Inflating ZLIB streams from io", "[util][pack]")
{
    io::BufferedIO io(
        "\x78\xDA\xCB\xC9\x4C\x4B\x55\xC8"
        "\x2C\x56\x48\xCE\x4F\x49\xE5\x02"
        "\x00\x20\xC1\x04\x62"_b);

    REQUIRE(zlib_inflate(io) == "life is code\n"_b);
    REQUIRE(io.eof());
}
