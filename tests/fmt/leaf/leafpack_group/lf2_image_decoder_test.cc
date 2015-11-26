#include "fmt/leaf/leafpack_group/lf2_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

static const std::string dir = "tests/fmt/leaf/files/lf2/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const Lf2ImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Leaf LF2 images", "[fmt]")
{
    do_test("C1F01.LF2", "C1F01-out.png");
}
