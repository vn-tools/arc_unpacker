#include "fmt/cronus/image_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::cronus;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    ImageConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("Decoding 8 bit Cronus images works")
{
    do_test(
        "tests/fmt/cronus/files/image/DPBG03D",
        "tests/fmt/cronus/files/image/DPBG03D-out.png");
}

TEST_CASE("Decoding 24 bit Cronus images works")
{
    do_test(
        "tests/fmt/cronus/files/image/TCF12",
        "tests/fmt/cronus/files/image/TCF12-out.png");
}
