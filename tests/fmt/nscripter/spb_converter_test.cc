#include "fmt/nscripter/spb_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::nscripter;

TEST_CASE("Decoding SPB-compressed stream works")
{
    SpbConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/nscripter/files/spb/grimoire_btn.bmp",
        "tests/fmt/nscripter/files/spb/grimoire_btn-out.png");
}
