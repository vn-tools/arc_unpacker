#include "fmt/bgi/cbg_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::bgi;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    CbgImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = decoder.decode(*input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("BGI CBG1 32-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/bgi/files/cbg-v1/ti_si_de_a1",
        "tests/fmt/bgi/files/cbg-v1/ti_si_de_a1-out.png");
}

TEST_CASE("BGI CBG1 24-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/bgi/files/cbg-v1/3",
        "tests/fmt/bgi/files/cbg-v1/3-out.png");
}

TEST_CASE("BGI CBG1 8-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/bgi/files/cbg-v1/4",
        "tests/fmt/bgi/files/cbg-v1/4-out.png");
}

TEST_CASE("BGI CBG2 32-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/bgi/files/cbg-v2/ms_wn_base",
        "tests/fmt/bgi/files/cbg-v2/ms_wn_base-out.png");
}

TEST_CASE("BGI CBG2 24-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/bgi/files/cbg-v2/l_card000",
        "tests/fmt/bgi/files/cbg-v2/l_card000-out.png");
}

TEST_CASE("BGI CBG2 8-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/bgi/files/cbg-v2/mask04r",
        "tests/fmt/bgi/files/cbg-v2/mask04r-out.png");
}
