#include "fmt/entis/eri_converter.h"
#include "test_support/catch.hh"
#include "test_support/image_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::entis;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    EriConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("Decoding ENTIS's lossless Huffman 32-bit non-flipped images works")
{
    do_test(
        "tests/fmt/entis/files/eri/img_rgba32.eri",
        "tests/fmt/entis/files/eri/img_rgba32-out.png");
}

TEST_CASE("Decoding ENTIS's lossless Gamma 32-bit flipped images works")
{
    do_test(
        "tests/fmt/entis/files/eri/cb10_14.eri",
        "tests/fmt/entis/files/eri/cb10_14-out.png");
}

TEST_CASE("Decoding ENTIS's lossless Nemesis 32-bit flipped images works")
{
    do_test(
        "tests/fmt/entis/files/eri/FRM_0201.eri",
        "tests/fmt/entis/files/eri/FRM_0201-out.png");
}

TEST_CASE("Decoding ENTIS's multi images works")
{
    do_test(
        "tests/fmt/entis/files/eri/FRM_0102.eri",
        "tests/fmt/entis/files/eri/FRM_0102-out.png");
}

TEST_CASE("Decoding ENTIS's 8-bit non-paletted images works")
{
    do_test(
        "tests/fmt/entis/files/eri/font24.eri",
        "tests/fmt/entis/files/eri/font24-out.png");
}
