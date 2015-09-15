#include "fmt/fvp/nvsg_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::fvp;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    NvsgConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("Decoding format 0 NVSG images works", "[fmt]")
{
    do_test(
        "tests/fmt/fvp/files/nvsg-0/BG085_001",
        "tests/fmt/fvp/files/nvsg-0/BG085_001-out.png");
}

TEST_CASE("Decoding format 1 NVSG images works", "[fmt]")
{
    do_test(
        "tests/fmt/fvp/files/nvsg-1/bu_effect22_F_1",
        "tests/fmt/fvp/files/nvsg-1/bu_effect22_F_1-out.png");
}

TEST_CASE("Decoding format 2 NVSG images works", "[fmt]")
{
    do_test(
        "tests/fmt/fvp/files/nvsg-2/CHR_時雨_基_夏私服_表情",
        "tests/fmt/fvp/files/nvsg-2/CHR_時雨_基_夏私服_表情-out.png");
}

TEST_CASE("Decoding format 3 NVSG images works", "[fmt]")
{
    do_test(
        "tests/fmt/fvp/files/nvsg-3/diss48",
        "tests/fmt/fvp/files/nvsg-3/diss48-out.png");
}

TEST_CASE("Decoding format 4 NVSG images works", "[fmt]")
{
    do_test(
        "tests/fmt/fvp/files/nvsg-4/gaiji_heart",
        "tests/fmt/fvp/files/nvsg-4/gaiji_heart-out.png");
}
