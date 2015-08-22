#include "fmt/rpgmaker/xyz_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::rpgmaker;

TEST_CASE("Decoding XYZ images works")
{
    XyzConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/rpgmaker/files/xyz/浅瀬部屋a.xyz",
        "tests/fmt/rpgmaker/files/xyz/浅瀬部屋a-out.png");
}
