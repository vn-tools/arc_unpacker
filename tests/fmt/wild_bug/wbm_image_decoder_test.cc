#include "fmt/wild_bug/wbm_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::wild_bug;

static const std::string dir = "tests/fmt/wild_bug/files/wbm/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const WbmImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Wild Bug WBM images", "[fmt]")
{
    SECTION("32-bit")
    {
        do_test("S_M01.WBM", "S_M01-out.png");
    }

    SECTION("24-bit")
    {
        do_test("S_Y08.WBM", "S_Y08-out.png");
    }

    SECTION("8-bit")
    {
        do_test("EF_WIPE_LR.WBM", "EF_WIPE_LR-out.png");
    }

    SECTION("External alpha channel")
    {
        do_test("ZD0211.WBM", "ZD0211-out.png");
    }

    SECTION("Unaligned stride")
    {
        do_test("ZD0710.WBM", "ZD0710-out.png");
    }

    SECTION("Transcription strategy 1")
    {
        do_test("ZD0710.WBM", "ZD0710-out.png");
    }

    SECTION("Transcription strategy 2")
    {
        do_test("S_M01.WBM", "S_M01-out.png");
    }

    SECTION("Retrieval strategy 1")
    {
        do_test("S_M01.WBM", "S_M01-out.png");
    }

    SECTION("Retrieval strategy 2")
    {
        do_test("E_AT06S.WBM", "E_AT06S-out.png");
    }

    SECTION("Retrieval strategy 3")
    {
        do_test("ZD0710.WBM", "ZD0710-out.png");
    }
}
