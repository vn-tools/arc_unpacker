#include "fmt/majiro/rct_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::majiro;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    RctImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = decoder.decode(*input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Majiro RCT v0 images", "[fmt]")
{
    do_test(
        "tests/fmt/majiro/files/rct/face_dummy.rct",
        "tests/fmt/majiro/files/rct/face_dummy-out.png");
}

TEST_CASE("Majiro RCT v1 images", "[fmt]")
{
    do_test(
        "tests/fmt/majiro/files/rct/ev04_01c.rct",
        "tests/fmt/majiro/files/rct/ev04_01c-out.png");
}
