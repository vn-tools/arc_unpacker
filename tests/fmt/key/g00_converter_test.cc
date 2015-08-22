#include "fmt/key/g00_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::key;

TEST_CASE("Decoding version 0 G00 images works")
{
    G00Converter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/key/files/g00-0/ayu_02.g00",
        "tests/fmt/key/files/g00-0/ayu_02-out.png");
}

TEST_CASE("Decoding version 1 G00 images works")
{
    G00Converter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/key/files/g00-1/ayu_05.g00",
        "tests/fmt/key/files/g00-1/ayu_05-out.png");
}

TEST_CASE("Decoding version 2 G00 images works")
{
    G00Converter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/key/files/g00-2/AYU_03.g00",
        "tests/fmt/key/files/g00-2/AYU_03-out.png");
}
