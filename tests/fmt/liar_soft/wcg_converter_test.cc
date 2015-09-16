#include "fmt/liar_soft/wcg_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::liar_soft;

TEST_CASE("LiarSoft WCG images", "[fmt]")
{
    WcgConverter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/liar_soft/files/wcg/0003.wcg");
    auto expected_image = tests::image_from_path(
        "tests/fmt/liar_soft/files/wcg/0003-out.png");
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}
