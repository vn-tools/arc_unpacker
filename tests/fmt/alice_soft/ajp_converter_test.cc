#include "fmt/alice_soft/ajp_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

TEST_CASE("Decoding transparent AJP images works")
{
    AjpConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/alice_soft/files/CG51478.ajp",
        "tests/fmt/alice_soft/files/CG51478-out.png");
}
