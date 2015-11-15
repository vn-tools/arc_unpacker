#include "fmt/west_vision/syg_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::west_vision;

static const std::string dir = "tests/fmt/west_vision/files/syg/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const SygImageDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::image_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_file);
}

TEST_CASE("WestVision SYG images", "[fmt]")
{
    do_test("loadx-zlib.syg", "loadx-out.png");
}
