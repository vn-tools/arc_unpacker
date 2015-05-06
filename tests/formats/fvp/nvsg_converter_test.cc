#include "formats/fvp/nvsg_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"
using namespace Formats::Fvp;

static void test_nvsg_decoding(
    const std::string &input_image_path,
    const std::string &expected_image_path)
{
    NvsgConverter converter;
    assert_decoded_image(converter, input_image_path, expected_image_path);
}

TEST_CASE("Decoding format 0 NVSG images works")
{
    test_nvsg_decoding(
        "tests/formats/fvp/files/BG085_001",
        "tests/formats/fvp/files/BG085_001-out.png");
}

TEST_CASE("Decoding format 1 NVSG images works")
{
    test_nvsg_decoding(
        "tests/formats/fvp/files/bu_effect22_F_1",
        "tests/formats/fvp/files/bu_effect22_F_1-out.png");
}

TEST_CASE("Decoding format 2 NVSG images works")
{
    test_nvsg_decoding(
        "tests/formats/fvp/files/CHR_時雨_基_夏私服_表情",
        "tests/formats/fvp/files/CHR_時雨_基_夏私服_表情-out.png");
}

TEST_CASE("Decoding format 3 NVSG images works")
{
    test_nvsg_decoding(
        "tests/formats/fvp/files/diss48",
        "tests/formats/fvp/files/diss48-out.png");
}

TEST_CASE("Decoding format 4 NVSG images works")
{
    test_nvsg_decoding(
        "tests/formats/fvp/files/gaiji_heart",
        "tests/formats/fvp/files/gaiji_heart-out.png");
}
