#include "fmt/entis/eri_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/image_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::entis;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    EriImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = decoder.decode(*input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Entis ERI lossless Huffman 32-bit non-flipped images", "[fmt]")
{
    do_test(
        "tests/fmt/entis/files/eri/img_rgba32.eri",
        "tests/fmt/entis/files/eri/img_rgba32-out.png");
}

TEST_CASE("Entis ERI lossless Gamma 32-bit flipped images", "[fmt]")
{
    do_test(
        "tests/fmt/entis/files/eri/cb10_14.eri",
        "tests/fmt/entis/files/eri/cb10_14-out.png");
}

TEST_CASE("Entis ERI lossless Nemesis 32-bit flipped images", "[fmt]")
{
    do_test(
        "tests/fmt/entis/files/eri/FRM_0201.eri",
        "tests/fmt/entis/files/eri/FRM_0201-out.png");
}

TEST_CASE("Entis ERI multi images", "[fmt]")
{
    do_test(
        "tests/fmt/entis/files/eri/FRM_0102.eri",
        "tests/fmt/entis/files/eri/FRM_0102-out.png");
}

TEST_CASE("Entis ERI 8-bit non-paletted images", "[fmt]")
{
    do_test(
        "tests/fmt/entis/files/eri/font24.eri",
        "tests/fmt/entis/files/eri/font24-out.png");
}
