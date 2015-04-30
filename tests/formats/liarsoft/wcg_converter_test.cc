#include "formats/liarsoft/wcg_converter.h"
#include "test_support/converter_support.h"
#include "test_support/catch.hpp"
using namespace Formats::LiarSoft;

TEST_CASE("Decoding WCG images works")
{
    WcgConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/liarsoft/files/0003.wcg",
        "tests/formats/liarsoft/files/0003-out.png");
}
