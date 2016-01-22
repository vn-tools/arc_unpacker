#include "dec/playstation/gim_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::playstation;

static const std::string dir = "tests/dec/playstation/files/gim/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = GimImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Playstation GIM images", "[dec]")
{
    SECTION("Pixel format 3, unswizzled")
    {
        do_test("fl_info.gim", "fl_info-out.png");
    }

    SECTION("Pixel format 3, swizzled")
    {
        do_test("fl_icons.gim", "fl_icons-out.png");
    }

    SECTION("Pixel format 4, palette format 3, swizzled")
    {
        do_test("sg_hint_good.gim", "sg_hint_good-out.png");
    }

    SECTION("Pixel format 5, palette format 3, swizzled")
    {
        do_test("bd_smenu.gim", "bd_smenu-out.png");
    }

    SECTION("Pixel format 5, palette format 3, unswizzled")
    {
        do_test("AY_6C_3.gim", "AY_6C_3-out.png");
    }
}
