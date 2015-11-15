#include "fmt/leaf/lf2_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

TEST_CASE("Leaf LF2 images", "[fmt]")
{
    const Lf2ImageDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/leaf/files/lf2/C1F01.LF2");
    const auto expected_file = tests::image_from_path(
        "tests/fmt/leaf/files/lf2/C1F01-out.png");
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_file);
}
