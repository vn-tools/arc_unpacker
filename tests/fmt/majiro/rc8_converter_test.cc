#include "fmt/majiro/rc8_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::majiro;

TEST_CASE("Decoding Majiro's RC8 images works")
{
    Rc8Converter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/majiro/files/rc8/style_tc_geo00_a_.rc8");
    auto expected_image = tests::image_from_path(
        "tests/fmt/majiro/files/rc8/style_tc_geo00_a_-out.png");
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image, false);
}
