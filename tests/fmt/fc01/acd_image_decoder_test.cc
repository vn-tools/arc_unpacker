#include "fmt/fc01/acd_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::fc01;

TEST_CASE("FC01 ACD monochrome images", "[fmt]")
{
    AcdImageDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/fc01/files/acd/CSD02_35.ACD");
    auto expected_image = tests::image_from_path(
        "tests/fmt/fc01/files/acd/CSD02_35-out.png");
    auto actual_image = decoder.decode(*input_file);
    tests::compare_images(*expected_image, actual_image);
}
