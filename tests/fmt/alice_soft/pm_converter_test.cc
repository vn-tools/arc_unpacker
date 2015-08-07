#include "fmt/alice_soft/pm_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"

using namespace au::fmt;
using namespace au::fmt::alice_soft;

static void test_pm_decoding(
    const std::string &input_image_path,
    const std::string &expected_image_path)
{
    PmConverter converter;
    au::tests::assert_decoded_image(
        converter, input_image_path, expected_image_path);
}

TEST_CASE("Decoding PM images works")
{
    test_pm_decoding(
        "tests/fmt/alice_soft/files/CG40000.pm",
        "tests/fmt/alice_soft/files/CG40000-out.png");
}
