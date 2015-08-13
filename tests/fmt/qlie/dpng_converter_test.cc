#include "fmt/qlie/dpng_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au::fmt::qlie;

TEST_CASE("Decoding DPNG images works")
{
    DpngConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/fmt/qlie/files/雷02.png",
        "tests/fmt/qlie/files/雷02-out.png");
}
