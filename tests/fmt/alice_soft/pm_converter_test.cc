#include "fmt/alice_soft/pm_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

TEST_CASE("Decoding PM images works")
{
    PmConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/alice_soft/files/pm/CG40000.pm",
        "tests/fmt/alice_soft/files/pm/CG40000-out.png");
}
