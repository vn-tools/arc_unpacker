#include "formats/lizsoft/sotes_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"
using namespace Formats::Lizsoft;

static void test_sotes_decoding(
    const std::string &input_image_path, const std::string expected_image_path)
{
    SotesConverter converter;
    assert_decoded_image(converter, input_image_path, expected_image_path);
}

TEST_CASE("Decoding RGB SOTES sprites works")
{
    test_sotes_decoding(
        "tests/formats/lizsoft/files/#1410",
        "tests/formats/lizsoft/files/#1410-out.png");
}

TEST_CASE("Decoding palette-based SOTES sprites works")
{
    test_sotes_decoding(
        "tests/formats/lizsoft/files/#1726",
        "tests/formats/lizsoft/files/#1726-out.png");
}
