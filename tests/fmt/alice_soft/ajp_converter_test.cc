#include "fmt/alice_soft/ajp_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au::fmt;
using namespace au::fmt::alice_soft;

static void test_ajp_decoding(
    const std::string &input_image_path,
    const std::string &expected_image_path)
{
    AjpConverter converter;
    au::tests::assert_decoded_image(
        converter, input_image_path, expected_image_path);
}

TEST_CASE("Decoding transparent AJP images works")
{
    test_ajp_decoding(
        "tests/fmt/alice_soft/files/CG51478.ajp",
        "tests/fmt/alice_soft/files/CG51478-out.png");
}
