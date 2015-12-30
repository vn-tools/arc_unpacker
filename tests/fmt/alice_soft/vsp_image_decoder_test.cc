#include "fmt/alice_soft/vsp_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const std::string dir = "tests/fmt/alice_soft/files/vsp/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = VspImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Alice Soft VSP images", "[fmt]")
{
    SECTION("VSP compression")
    {
        do_test("CG8367.VSP", "CG8367-out.png");
    }

    SECTION("PMS compression, monochrome")
    {
        do_test("CG_0295.VSP", "CG_0295-out.png");
    }

    SECTION(
        "PMS compression, true color palette")
    {
        do_test("045.vsp", "045-out.png");
    }
}
