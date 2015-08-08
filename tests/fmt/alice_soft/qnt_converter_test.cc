#include "fmt/alice_soft/qnt_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"

using namespace au::fmt;
using namespace au::fmt::alice_soft;

static void test_qnt_decoding(
    const std::string &input_image_path,
    const std::string &expected_image_path)
{
    QntConverter converter;
    au::tests::assert_decoded_image(
        converter, input_image_path, expected_image_path);
}

TEST_CASE("Decoding opaque QNT images works")
{
    test_qnt_decoding(
        "tests/fmt/alice_soft/files/CG00505.QNT",
        "tests/fmt/alice_soft/files/CG00505-out.png");
}


TEST_CASE("Decoding transparent QNT images with size divisible by 2 works")
{
    test_qnt_decoding(
        "tests/fmt/alice_soft/files/CG64100.QNT",
        "tests/fmt/alice_soft/files/CG64100-out.png");
}

TEST_CASE(
    "Decoding transparent QNT images works with size not divisible by 2 works")
{
    test_qnt_decoding(
        "tests/fmt/alice_soft/files/CG64214.QNT",
        "tests/fmt/alice_soft/files/CG64214-out.png");
}
