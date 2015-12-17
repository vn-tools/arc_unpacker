#include "fmt/kiss/custom_png_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::kiss;

static const std::string dir = "tests/fmt/kiss/files/custom-png/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const CustomPngImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Kiss custom PNG images", "[fmt]")
{
    do_test("x04n_a_.png", "x04n_a_-out.png");
}
