#include "fmt/majiro/rc8_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::majiro;

TEST_CASE("Majiro RC8 images", "[fmt]")
{
    Rc8ImageDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/majiro/files/rc8/style_tc_geo00_a_.rc8");
    auto expected_image = tests::image_from_path(
        "tests/fmt/majiro/files/rc8/style_tc_geo00_a_-out.png");
    auto actual_image = decoder.decode(*input_file);
    tests::compare_images(*expected_image, actual_image);
}
