#include "fmt/majiro/rct_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::majiro;

TEST_CASE("Decoding Majiro's RCT images works")
{
    RctConverter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/majiro/files/rct/face_dummy.rct");
    auto expected_image = tests::image_from_path(
        "tests/fmt/majiro/files/rct/face_dummy-out.png");
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image, false);
}
