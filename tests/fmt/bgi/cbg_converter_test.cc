#include "fmt/bgi/cbg_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::bgi;

static void do_test(
    const std::string &input_path,
    const std::string &expected_path,
    int max_delta = 0)
{
    CbgConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image, max_delta);
}

TEST_CASE("Decoding 32-bit CBG1 images works")
{
    do_test(
        "tests/fmt/bgi/files/cbg-v1/ti_si_de_a1",
        "tests/fmt/bgi/files/cbg-v1/ti_si_de_a1-out.png");
}

TEST_CASE("Decoding 24-bit CBG1 images works")
{
    do_test(
        "tests/fmt/bgi/files/cbg-v1/3",
        "tests/fmt/bgi/files/cbg-v1/3-out.png");
}

TEST_CASE("Decoding 8-bit CBG1 images works")
{
    do_test(
        "tests/fmt/bgi/files/cbg-v1/4",
        "tests/fmt/bgi/files/cbg-v1/4-out.png");
}

TEST_CASE("Decoding 32-bit CBG2 images works")
{
    do_test(
        "tests/fmt/bgi/files/cbg-v2/ms_wn_base",
        "tests/fmt/bgi/files/cbg-v2/ms_wn_base-out.png",
        2);
}

TEST_CASE("Decoding 24-bit CBG2 images works")
{
    do_test(
        "tests/fmt/bgi/files/cbg-v2/l_card000",
        "tests/fmt/bgi/files/cbg-v2/l_card000-out.png",
        2);
}

TEST_CASE("Decoding 8-bit CBG2 images works")
{
    do_test(
        "tests/fmt/bgi/files/cbg-v2/mask04r",
        "tests/fmt/bgi/files/cbg-v2/mask04r-out.png",
        2);
}
