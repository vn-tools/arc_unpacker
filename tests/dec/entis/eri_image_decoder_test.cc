#include "dec/entis/eri_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::entis;

static const std::string dir = "tests/dec/entis/files/eri/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = EriImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Entis ERI images", "[dec]")
{
    SECTION("Huffman, 32-bit, non-flipped")
    {
        do_test("img_rgba32.eri", "img_rgba32-out.png");
    }

    SECTION("Gamma, 32-bit, flipped")
    {
        do_test("cb10_14.eri", "cb10_14-out.png");
    }

    SECTION("Nemesis, 32-bit, flipped")
    {
        do_test("FRM_0201.eri", "FRM_0201-out.png");
    }

    SECTION("Multi images")
    {
        do_test("FRM_0102.eri", "FRM_0102-out.png");
    }

    SECTION("8-bit, non-paletted")
    {
        do_test("font24.eri", "font24-out.png");
    }
}
