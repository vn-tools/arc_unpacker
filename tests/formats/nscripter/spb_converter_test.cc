#include "formats/nscripter/spb_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"
using namespace Formats::NScripter;

TEST_CASE("Decoding SPB-compressed stream works")
{
    SpbConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/nscripter/files/grimoire_btn.bmp",
        "tests/formats/nscripter/files/grimoire_btn-out.png");
}
