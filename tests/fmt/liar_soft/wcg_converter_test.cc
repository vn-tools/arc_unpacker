#include "fmt/liar_soft/wcg_converter.h"
#include "test_support/converter_support.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt::liar_soft;

TEST_CASE("Decoding WCG images works")
{
    WcgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/liar_soft/files/wcg/0003.wcg",
        "tests/fmt/liar_soft/files/wcg/0003-out.png");
}
