#include "fmt/cronus/grp_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::cronus;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    GrpImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = decoder.decode(*input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Cronus GRP 8-bit images (DokiDoki Princess)", "[fmt]")
{
    do_test(
        "tests/fmt/cronus/files/grp/DPBG03D",
        "tests/fmt/cronus/files/grp/DPBG03D-out.png");
}

TEST_CASE("Cronus GRP 24-bit images (DokiDoki Princess)", "[fmt]")
{
    do_test(
        "tests/fmt/cronus/files/grp/TCF12",
        "tests/fmt/cronus/files/grp/TCF12-out.png");
}

TEST_CASE("Cronus GRP 24-bit images (Sweet Pleasure)", "[fmt]")
{
    do_test(
        "tests/fmt/cronus/files/grp/MSGPARTS",
        "tests/fmt/cronus/files/grp/MSGPARTS-out.png");
}

TEST_CASE("Cronus GRP 8-bit v2 images (Nursery Song)", "[fmt]")
{
    do_test(
        "tests/fmt/cronus/files/grp/SR1",
        "tests/fmt/cronus/files/grp/SR1-out.png");
}

TEST_CASE("Cronus GRP 32-bit v2 images (Nursery Song)", "[fmt]")
{
    do_test(
        "tests/fmt/cronus/files/grp/SELWIN",
        "tests/fmt/cronus/files/grp/SELWIN-out.png");
}
