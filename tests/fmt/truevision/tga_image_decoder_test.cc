#include "fmt/truevision/tga_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::truevision;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const TgaImageDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_file = tests::image_from_path(expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_file);
}

TEST_CASE("Truevision TGA images (8-bit, compressed)", "[fmt]")
{
    do_test(
        "tests/fmt/truevision/files/tga/compressed-8-bit.tga",
        "tests/fmt/truevision/files/tga/compressed-8-bit-out.png");
}

TEST_CASE("Truevision TGA images (16-bit, compressed)", "[fmt]")
{
    do_test(
        "tests/fmt/truevision/files/tga/compressed-16-bit.tga",
        "tests/fmt/truevision/files/tga/compressed-16-bit-out.png");
}

TEST_CASE("Truevision TGA images (24-bit, compressed)", "[fmt]")
{
    do_test(
        "tests/fmt/truevision/files/tga/compressed-24-bit.tga",
        "tests/fmt/truevision/files/tga/compressed-24-bit-out.png");
}

TEST_CASE("Truevision TGA images (32-bit, compressed)", "[fmt]")
{
    do_test(
        "tests/fmt/truevision/files/tga/compressed-32-bit.tga",
        "tests/fmt/truevision/files/tga/compressed-32-bit-out.png");
}

TEST_CASE("Truevision TGA images (8-bit, uncompressed, palette)", "[fmt]")
{
    do_test(
        "tests/fmt/truevision/files/tga/uncompressed-8-bit-palette.tga",
        "tests/fmt/truevision/files/tga/uncompressed-8-bit-palette-out.png");
}

TEST_CASE("Truevision TGA images (16-bit, uncompressed)", "[fmt]")
{
    do_test(
        "tests/fmt/truevision/files/tga/uncompressed-16-bit.tga",
        "tests/fmt/truevision/files/tga/uncompressed-16-bit-out.png");
}

TEST_CASE("Truevision TGA images (24-bit, uncompressed)", "[fmt]")
{
    do_test(
        "tests/fmt/truevision/files/tga/uncompressed-24-bit.tga",
        "tests/fmt/truevision/files/tga/uncompressed-24-bit-out.png");
}

TEST_CASE("Truevision TGA images (32-bit, uncompressed)", "[fmt]")
{
    do_test(
        "tests/fmt/truevision/files/tga/uncompressed-32-bit.tga",
        "tests/fmt/truevision/files/tga/uncompressed-32-bit-out.png");
}

TEST_CASE("Truevision TGA images (16-bit, uncompressed, flipped)", "[fmt]")
{
    do_test(
        "tests/fmt/truevision/files/tga/uncompressed-16-bit-flipped.tga",
        "tests/fmt/truevision/files/tga/uncompressed-16-bit-flipped-out.png");
}
