#include "fmt/fvp/nvsg_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au::fmt::fvp;

static void test_nvsg_decoding(
    const std::string &input_image_path,
    const std::string &expected_image_path)
{
    NvsgConverter converter;
    au::tests::assert_decoded_image(
        converter, input_image_path, expected_image_path);
}

TEST_CASE("Decoding format 0 NVSG images works")
{
    test_nvsg_decoding(
        "tests/fmt/fvp/files/BG085_001",
        "tests/fmt/fvp/files/BG085_001-out.png");
}

TEST_CASE("Decoding format 1 NVSG images works")
{
    test_nvsg_decoding(
        "tests/fmt/fvp/files/bu_effect22_F_1",
        "tests/fmt/fvp/files/bu_effect22_F_1-out.png");
}

TEST_CASE("Decoding format 2 NVSG images works")
{
    test_nvsg_decoding(
        "tests/fmt/fvp/files/CHR_時雨_基_夏私服_表情",
        "tests/fmt/fvp/files/CHR_時雨_基_夏私服_表情-out.png");
}

TEST_CASE("Decoding format 3 NVSG images works")
{
    test_nvsg_decoding(
        "tests/fmt/fvp/files/diss48",
        "tests/fmt/fvp/files/diss48-out.png");
}

TEST_CASE("Decoding format 4 NVSG images works")
{
    test_nvsg_decoding(
        "tests/fmt/fvp/files/gaiji_heart",
        "tests/fmt/fvp/files/gaiji_heart-out.png");
}
