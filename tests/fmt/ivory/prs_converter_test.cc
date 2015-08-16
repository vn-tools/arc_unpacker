#include "fmt/ivory/prs_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::ivory;

TEST_CASE("Decoding PRS images works")
{
    PrsConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/ivory/files/BMIK_A16",
        "tests/fmt/ivory/files/BMIK_A16-out.png");
}
