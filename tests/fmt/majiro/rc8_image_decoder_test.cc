#include "fmt/majiro/rc8_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::majiro;

TEST_CASE("Majiro RC8 images", "[fmt]")
{
    const Rc8ImageDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/majiro/files/rc8/style_tc_geo00_a_.rc8");
    const auto expected_image = tests::image_from_path(
        "tests/fmt/majiro/files/rc8/style_tc_geo00_a_-out.png");
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}
