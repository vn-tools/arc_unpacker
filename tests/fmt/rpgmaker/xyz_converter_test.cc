#include "fmt/rpgmaker/xyz_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::rpgmaker;

TEST_CASE("Decoding XYZ images works")
{
    XyzConverter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/rpgmaker/files/xyz/浅瀬部屋a.xyz");
    auto expected_image = tests::image_from_path(
        "tests/fmt/rpgmaker/files/xyz/浅瀬部屋a-out.png");
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}
