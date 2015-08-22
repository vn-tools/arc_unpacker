#include "fmt/kid/prt_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::kid;

TEST_CASE("Decoding PRT images works")
{
    PrtConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/kid/files/prt/bg01a1.prt",
        "tests/fmt/kid/files/prt/bg01a1-out.png");
}

TEST_CASE("Decoding PRT images with alpha channel works")
{
    PrtConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/kid/files/cps/yh04adm.prt",
        "tests/fmt/kid/files/prt/yh04adm-out.png");
}

TEST_CASE("Decoding 8-bit PRT images works")
{
    PrtConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/kid/files/prt/saver_sm.prt",
        "tests/fmt/kid/files/prt/saver_sm-out.png");
}
