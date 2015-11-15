#include "fmt/wild_bug/wbm_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::wild_bug;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const WbmImageDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_file = tests::image_from_path(expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_file);
}

TEST_CASE("Wild Bug WBM 32-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/wild_bug/files/wbm/S_M01.WBM",
        "tests/fmt/wild_bug/files/wbm/S_M01-out.png");
}

TEST_CASE("Wild Bug WBM 24-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/wild_bug/files/wbm/S_Y08.WBM",
        "tests/fmt/wild_bug/files/wbm/S_Y08-out.png");
}

TEST_CASE("Wild Bug WBM 8-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/wild_bug/files/wbm/EF_WIPE_LR.WBM",
        "tests/fmt/wild_bug/files/wbm/EF_WIPE_LR-out.png");
}

TEST_CASE("Wild Bug WBM images with external alpha channel", "[fmt]")
{
    do_test(
        "tests/fmt/wild_bug/files/wbm/ZD0211.WBM",
        "tests/fmt/wild_bug/files/wbm/ZD0211-out.png");
}

TEST_CASE("Wild Bug WBM images with unaligned stride", "[fmt]")
{
    do_test(
        "tests/fmt/wild_bug/files/wbm/ZD0710.WBM",
        "tests/fmt/wild_bug/files/wbm/ZD0710-out.png");
}

TEST_CASE("Wild Bug WBM images using transcription strategy 1", "[fmt]")
{
    do_test(
        "tests/fmt/wild_bug/files/wbm/ZD0710.WBM",
        "tests/fmt/wild_bug/files/wbm/ZD0710-out.png");
}

TEST_CASE("Wild Bug WBM images using transcription strategy 2", "[fmt]")
{
    do_test(
        "tests/fmt/wild_bug/files/wbm/S_M01.WBM",
        "tests/fmt/wild_bug/files/wbm/S_M01-out.png");
}

TEST_CASE("Wild Bug WBM images using retrieval strategy 1", "[fmt]")
{
    do_test(
        "tests/fmt/wild_bug/files/wbm/S_M01.WBM",
        "tests/fmt/wild_bug/files/wbm/S_M01-out.png");
}

TEST_CASE("Wild Bug WBM images using retrieval strategy 2", "[fmt]")
{
    do_test(
        "tests/fmt/wild_bug/files/wbm/E_AT06S.WBM",
        "tests/fmt/wild_bug/files/wbm/E_AT06S-out.png");
}

TEST_CASE("Wild Bug WBM images using retrieval strategy 3", "[fmt]")
{
    do_test(
        "tests/fmt/wild_bug/files/wbm/ZD0710.WBM",
        "tests/fmt/wild_bug/files/wbm/ZD0710-out.png");
}
