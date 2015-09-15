#include "fmt/yuka_script/ykg_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::yuka_script;

TEST_CASE("Decoding YKG images works", "[fmt]")
{
    YkgConverter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/yuka_script/files/ykg/reimu.ykg");
    auto expected_image = tests::image_from_path(
        "tests/fmt/yuka_script/files/ykg/reimu-out.png");
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}
