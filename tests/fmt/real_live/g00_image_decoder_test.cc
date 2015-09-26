#include "fmt/real_live/g00_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::real_live;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    G00ImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*decoder.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("RealLive G00 v0 images", "[fmt]")
{
    do_test(
        "tests/fmt/real_live/files/g00-0/ayu_02.g00",
        "tests/fmt/real_live/files/g00-0/ayu_02-out.png");
}

TEST_CASE("RealLive G00 v1 images", "[fmt]")
{
    do_test(
        "tests/fmt/real_live/files/g00-1/ayu_05.g00",
        "tests/fmt/real_live/files/g00-1/ayu_05-out.png");
}

TEST_CASE("RealLive G00 v2 images", "[fmt]")
{
    do_test(
        "tests/fmt/real_live/files/g00-2/AYU_03.g00",
        "tests/fmt/real_live/files/g00-2/AYU_03-out.png");
}
