#include "formats/yukascript/ykg_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"
using namespace Formats::YukaScript;

TEST_CASE("Decoding YKG images works")
{
    YkgConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/yukascript/files/reimu.ykg",
        "tests/formats/yukascript/files/reimu-out.png");
}
