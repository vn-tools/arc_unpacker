#include "fmt/leaf/lf3_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

TEST_CASE("Leaf LF3 images", "[fmt]")
{
    Lf3ImageDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/leaf/files/lf3/KEY2.LF3");
    auto expected_file = tests::image_from_path(
        "tests/fmt/leaf/files/lf3/KEY2-out.png");
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_file);
}
