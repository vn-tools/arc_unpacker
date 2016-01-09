#include "dec/lucifen/elg_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::lucifen;

static const std::string dir = "tests/dec/lucifen/files/elg/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = ElgImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Lucifen ELG images", "[dec]")
{
    SECTION("8-bit")
    {
        do_test("rule006.elg", "rule006-out.png");
    }

    SECTION("24-bit")
    {
        do_test("rf_bg.elg", "rf_bg-out.png");
    }

    SECTION("32-bit opaque")
    {
        do_test("if_lb.elg", "if_lb-out.png");
    }

    SECTION("32-bit transparent")
    {
        do_test("tl_ref.elg", "tl_ref-out.png");
    }
}
