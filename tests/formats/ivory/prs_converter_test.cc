#include "formats/ivory/prs_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"
using namespace Formats::Ivory;

TEST_CASE("Decoding PRS images works")
{
    PrsConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/ivory/files/BMIK_A16",
        "tests/formats/ivory/files/BMIK_A16-out.png");
}
