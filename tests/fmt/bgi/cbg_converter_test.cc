#include "fmt/bgi/cbg_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"

using namespace au::fmt;
using namespace au::fmt::bgi;

static void test_cbg_decoding(
    const std::string &input_image_path,
    const std::string &expected_image_path)
{
    CbgConverter converter;
    au::tests::assert_decoded_image(
        converter, input_image_path, expected_image_path);
}

TEST_CASE("Decoding 24-bit CBG images works")
{
    test_cbg_decoding(
        "tests/fmt/bgi/files/3",
        "tests/fmt/bgi/files/3-out.png");
}

TEST_CASE("Decoding 8-bit CBG images works")
{
    test_cbg_decoding(
        "tests/fmt/bgi/files/4",
        "tests/fmt/bgi/files/4-out.png");
}

TEST_CASE("Decoding 32-bit CBG images works")
{
    test_cbg_decoding(
        "tests/fmt/bgi/files/ti_si_de_a1",
        "tests/fmt/bgi/files/ti_si_de_a1-out.png");
}
