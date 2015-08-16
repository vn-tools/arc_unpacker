#include "fmt/bgi/cbg_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::bgi;

TEST_CASE("Decoding 24-bit CBG images works")
{
    CbgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/bgi/files/3",
        "tests/fmt/bgi/files/3-out.png");
}

TEST_CASE("Decoding 8-bit CBG images works")
{
    CbgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/bgi/files/4",
        "tests/fmt/bgi/files/4-out.png");
}

TEST_CASE("Decoding 32-bit CBG images works")
{
    CbgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/bgi/files/ti_si_de_a1",
        "tests/fmt/bgi/files/ti_si_de_a1-out.png");
}
