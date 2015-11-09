#include "fmt/kiss/custom_png_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::kiss;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    CustomPngImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_file = tests::image_from_path(expected_path);
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_file);
}

TEST_CASE("Kiss custom PNG images", "[fmt]")
{
    do_test(
        "tests/fmt/kiss/files/custom-png/x04n_a_.png",
        "tests/fmt/kiss/files/custom-png/x04n_a_-out.png");
}
