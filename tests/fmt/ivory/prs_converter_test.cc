#include "fmt/ivory/prs_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"

using namespace au::fmt::ivory;

TEST_CASE("Decoding PRS images works")
{
    PrsConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/fmt/ivory/files/BMIK_A16",
        "tests/fmt/ivory/files/BMIK_A16-out.png");
}
