#include "dec/bgi/cbg_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::bgi;

static const std::string dir = "tests/dec/bgi/files/cbg/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = CbgImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("BGI CBG images", "[dec]")
{
    SECTION("Version 1, 8-bit")
    {
        do_test("v1/4", "v1/4-out.png");
    }

    SECTION("Version 1, 24-bit")
    {
        do_test("v1/3", "v1/3-out.png");
    }

    SECTION("Version 1, 32-bit")
    {
        do_test("v1/ti_si_de_a1", "v1/ti_si_de_a1-out.png");
    }

    SECTION("Version 2, 8-bit")
    {
        do_test("v2/mask04r", "v2/mask04r-out.png");
    }

    SECTION("Version 2, 24-bit")
    {
        do_test("v2/l_card000", "v2/l_card000-out.png");
    }

    SECTION("Version 2, 32-bit")
    {
        do_test("v2/ms_wn_base", "v2/ms_wn_base-out.png");
    }
}
