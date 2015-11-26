#include "fmt/liar_soft/lim_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::liar_soft;

static const std::string dir = "tests/fmt/liar_soft/files/lim/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const LimImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("LiarSoft LIM images", "[fmt]")
{
    SECTION("Opaque")
    {
        do_test("MU_ST.lim", "MU_ST-out.png");
    }

    SECTION("Transparent")
    {
        do_test("7035.lim", "7035-out.png");
    }
}
