#include "dec/fc01/mcg_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::fc01;

static const std::string dir = "tests/dec/fc01/files/mcg/";

static void do_test(
    const std::string &input_path,
    const std::string &expected_path,
    const u8 key)
{
    McgImageDecoder decoder;
    decoder.key = key;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("FC01 MCG images", "[dec]")
{
    SECTION("v1.01, 24-bit, aligned")
    {
        do_test("FC01LOGO.MCG", "FC01LOGO-out.png", 209);
    }

    SECTION("v1.01, 24-bit, unaligned")
    {
        do_test("CLE13_06.MCG", "CLE13_06-out.png", 209);
    }

    SECTION("v2.00, 24-bit")
    {
        do_test("50.MCG", "50-out.png", 77);
    }
}
