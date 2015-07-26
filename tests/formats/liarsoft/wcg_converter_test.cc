#include "formats/liarsoft/wcg_converter.h"
#include "test_support/converter_support.h"
#include "test_support/catch.hpp"

using namespace au::fmt::liarsoft;

TEST_CASE("Decoding WCG images works")
{
    WcgConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/formats/liarsoft/files/0003.wcg",
        "tests/formats/liarsoft/files/0003-out.png");
}
