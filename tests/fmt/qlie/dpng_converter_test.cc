#include "fmt/qlie/dpng_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::qlie;

TEST_CASE("Decoding DPNG images works")
{
    DpngConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/qlie/files/dpng/雷02.png",
        "tests/fmt/qlie/files/dpng/雷02-out.png");
}
