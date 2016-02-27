#include "algo/pack/zlib.h"
#include "io/memory_stream.h"
#include "test_support/catch.h"
#include "test_support/common.h"

using namespace au;
using namespace au::algo::pack;

TEST_CASE("ZLIB compression", "[algo][pack]")
{
    const bstr input =
        "\x78\xDA\xCB\xC9\x4C\x4B\x55\xC8"
        "\x2C\x56\x48\xCE\x4F\x49\xE5\x02"
        "\x00\x20\xC1\x04\x62"_b;
    const bstr output = "life is code\n"_b;

    SECTION("Inflating ZLIB from bstr")
    {
        tests::compare_binary(zlib_inflate(input), output);
    }

    SECTION("Inflating ZLIB from stream")
    {
        io::MemoryStream input_stream(input);
        tests::compare_binary(zlib_inflate(input_stream), output);
        REQUIRE(input_stream.left() == 0);
    }

    SECTION("Deflating ZLIB from bstr")
    {
        tests::compare_binary(zlib_inflate(zlib_deflate(output)), output);
    }

    SECTION("Deflating ZLIB with RawDeflate")
    {
        const auto deflated = zlib_deflate(output, ZlibKind::RawDeflate);
        const auto inflated = zlib_inflate(deflated, ZlibKind::RawDeflate);
        tests::compare_binary(inflated, output);
    }
}
