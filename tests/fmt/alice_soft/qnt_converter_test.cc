#include "fmt/alice_soft/qnt_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

TEST_CASE("Decoding opaque QNT images works")
{
    QntConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/alice_soft/files/CG00505.QNT",
        "tests/fmt/alice_soft/files/CG00505-out.png");
}

TEST_CASE("Decoding transparent QNT images with size divisible by 2 works")
{
    QntConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/alice_soft/files/CG64100.QNT",
        "tests/fmt/alice_soft/files/CG64100-out.png");
}

TEST_CASE(
    "Decoding transparent QNT images works with size not divisible by 2 works")
{
    QntConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/alice_soft/files/CG64214.QNT",
        "tests/fmt/alice_soft/files/CG64214-out.png");
}
