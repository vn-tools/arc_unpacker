#include "fmt/playstation/gim_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::playstation;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const GimImageDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_image = tests::image_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Playstation GIM images (pixel format 3, unswizzled)", "[fmt]")
{
    do_test(
        "tests/fmt/playstation/files/gim/fl_info.gim",
        "tests/fmt/playstation/files/gim/fl_info-out.png");
}

TEST_CASE("Playstation GIM images (pixel format 3, swizzled)", "[fmt]")
{
    do_test(
        "tests/fmt/playstation/files/gim/fl_icons.gim",
        "tests/fmt/playstation/files/gim/fl_icons-out.png");
}

TEST_CASE(
    "Playstation GIM images (pixel format 4, palette format 3, swizzled)",
    "[fmt]")
{
    do_test(
        "tests/fmt/playstation/files/gim/sg_hint_good.gim",
        "tests/fmt/playstation/files/gim/sg_hint_good-out.png");
}

TEST_CASE(
    "Playstation GIM images (pixel format 5, palette format 3, swizzled)",
    "[fmt]")
{
    do_test(
        "tests/fmt/playstation/files/gim/bd_smenu.gim",
        "tests/fmt/playstation/files/gim/bd_smenu-out.png");
}

TEST_CASE(
    "Playstation GIM images (pixel format 5, palette format 3, unswizzled)",
    "[fmt]")
{
    do_test(
        "tests/fmt/playstation/files/gim/AY_6C_3.gim",
        "tests/fmt/playstation/files/gim/AY_6C_3-out.png");
}
