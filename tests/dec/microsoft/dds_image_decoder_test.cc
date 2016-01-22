#include "dec/microsoft/dds_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::microsoft;

static const std::string dir = "tests/dec/microsoft/files/dds/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = DdsImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Microsoft DDS textures", "[dec]")
{
    SECTION("DXT1")
    {
        do_test("back0.dds", "back0-out.png");
    }

    SECTION("DXT3")
    {
        do_test("006_disconnect.dds", "006_disconnect-out.png");
    }

    SECTION("DXT5")
    {
        do_test("reimu1.dds", "reimu1-out.png");
    }

    SECTION("Raw 32-bit")
    {
        do_test("koishi_7.dds", "koishi_7-out.png");
    }
}
