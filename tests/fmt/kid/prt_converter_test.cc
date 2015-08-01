#include "fmt/kid/prt_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"

using namespace au::fmt::kid;

TEST_CASE("Decoding PRT images works")
{
    PrtConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/fmt/kid/files/bg01a1.prt",
        "tests/fmt/kid/files/bg01a1-out.png");
}

TEST_CASE("Decoding PRT images with alpha channel works")
{
    PrtConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/fmt/kid/files/yh04adm.prt",
        "tests/fmt/kid/files/yh04adm-out.png");
}
