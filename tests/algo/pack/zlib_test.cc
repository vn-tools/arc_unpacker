#include "algo/pack/zlib.h"
#include "io/memory_stream.h"
#include "test_support/catch.h"

using namespace au;
using namespace au::algo::pack;

TEST_CASE("ZLIB unpacking", "[algo][pack]")
{
    SECTION("Inflating ZLIB from bstr")
    {
        const bstr input =
            "\x78\xDA\xCB\xC9\x4C\x4B\x55\xC8"
            "\x2C\x56\x48\xCE\x4F\x49\xE5\x02"
            "\x00\x20\xC1\x04\x62"_b;

        REQUIRE(zlib_inflate(input) == "life is code\n"_b);
    }

    SECTION("Inflating ZLIB from stream")
    {
        io::MemoryStream stream(
            "\x78\xDA\xCB\xC9\x4C\x4B\x55\xC8"
            "\x2C\x56\x48\xCE\x4F\x49\xE5\x02"
            "\x00\x20\xC1\x04\x62"_b);

        REQUIRE(zlib_inflate(stream) == "life is code\n"_b);
        REQUIRE(stream.eof());
    }
}
