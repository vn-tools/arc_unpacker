#include "fmt/liar_soft/lim_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::liar_soft;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const LimImageDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_image = tests::image_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("LiarSoft LIM transparent images", "[fmt]")
{
    do_test(
        "tests/fmt/liar_soft/files/lim/7035.lim",
        "tests/fmt/liar_soft/files/lim/7035-out.png");
}

TEST_CASE("LiarSoft LIM opaque images", "[fmt]")
{
    do_test(
        "tests/fmt/liar_soft/files/lim/MU_ST.lim",
        "tests/fmt/liar_soft/files/lim/MU_ST-out.png");
}
