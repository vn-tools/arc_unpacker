#include "fmt/cronus/grp_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::cronus;

static const std::string dir = "tests/fmt/cronus/files/grp/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const GrpImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Cronus GRP images", "[fmt]")
{
    SECTION("Version 1, 8-bit (DokiDoki Princess)")
    {
        do_test("DPBG03D", "DPBG03D-out.png");
    }

    SECTION("Version 1, 24-bit (DokiDoki Princess)")
    {
        do_test("TCF12", "TCF12-out.png");
    }

    SECTION("Version 1, 24-bit (Sweet Pleasure)")
    {
        do_test("MSGPARTS", "MSGPARTS-out.png");
    }

    SECTION("Version 2, 8-bit (Nursery Song)")
    {
        do_test("SR1", "SR1-out.png");
    }

    SECTION("Version 2, 32-bit (Nursery Song)")
    {
        do_test("SELWIN", "SELWIN-out.png");
    }
}
