#include "fmt/lizsoft/sotes_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::lizsoft;

TEST_CASE("Decoding RGB SOTES sprites works")
{
    SotesConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/lizsoft/files/#1410",
        "tests/fmt/lizsoft/files/#1410-out.png");
}

TEST_CASE("Decoding palette-based SOTES sprites works")
{
    SotesConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/lizsoft/files/#1726",
        "tests/fmt/lizsoft/files/#1726-out.png");
}
