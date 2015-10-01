#include "fmt/microsoft/bmp_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::microsoft;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    BmpImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = decoder.decode(*input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Microsoft BMP images with 1-bit palette", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/pal1bg.bmp",
        "tests/fmt/microsoft/files/bmp/pal1bg-out.png");
}

TEST_CASE("Microsoft BMP images with 2-bit palette", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/pal2.bmp",
        "tests/fmt/microsoft/files/bmp/pal2-out.png");
}

TEST_CASE("Microsoft BMP images with 4-bit palette", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/pal4.bmp",
        "tests/fmt/microsoft/files/bmp/pal4-out.png");
}

TEST_CASE("Microsoft BMP images with 8-bit palette", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/pal8.bmp",
        "tests/fmt/microsoft/files/bmp/pal8-out.png");
}

TEST_CASE("Microsoft BMP images with implied 8-bit palette", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/pal8-0.bmp",
        "tests/fmt/microsoft/files/bmp/pal8-out.png");
}

TEST_CASE("Microsoft BMP v4 images with 8-bit palette", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/pal8v4.bmp",
        "tests/fmt/microsoft/files/bmp/pal8-out.png");
}

TEST_CASE("Microsoft BMP v5 images with 8-bit palette", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/pal8v5.bmp",
        "tests/fmt/microsoft/files/bmp/pal8-out.png");
}

TEST_CASE("Microsoft BMP 16-bit images (555X)", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/rgb16.bmp",
        "tests/fmt/microsoft/files/bmp/rgb16-out.png");
}

TEST_CASE("Microsoft BMP 16-bit images (565)", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/rgb16-565.bmp",
        "tests/fmt/microsoft/files/bmp/rgb16-565-out.png");
}

TEST_CASE("Microsoft BMP 16-bit images (231)", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/rgb16-231.bmp",
        "tests/fmt/microsoft/files/bmp/rgb16-231-out.png");
}

TEST_CASE("Microsoft BMP 16-bit images (4444)", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/rgba16-4444.bmp",
        "tests/fmt/microsoft/files/bmp/rgba16-4444-out.png");
}

TEST_CASE("Microsoft BMP 24-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/rgb24.bmp",
        "tests/fmt/microsoft/files/bmp/rgb24-out.png");
}

TEST_CASE("Microsoft BMP 24-bit images with fake palette", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/rgb24pal.bmp",
        "tests/fmt/microsoft/files/bmp/rgb24-out.png");
}

TEST_CASE("Microsoft BMP 32-bit images without alpha", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/rgb32.bmp",
        "tests/fmt/microsoft/files/bmp/rgb32-out.png");
}

TEST_CASE("Microsoft BMP 32-bit images with alpha", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/rgba32.bmp",
        "tests/fmt/microsoft/files/bmp/rgba32-out.png");
}

TEST_CASE("Microsoft BMP 32-bit images with fake alpha", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/rgb32fakealpha.bmp",
        "tests/fmt/microsoft/files/bmp/rgb32fakealpha-out.png");
}

TEST_CASE("Microsoft BMP unaligned images", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/pal8w126.bmp",
        "tests/fmt/microsoft/files/bmp/pal8w126-out.png");
    do_test(
        "tests/fmt/microsoft/files/bmp/pal8w125.bmp",
        "tests/fmt/microsoft/files/bmp/pal8w125-out.png");
    do_test(
        "tests/fmt/microsoft/files/bmp/pal8w124.bmp",
        "tests/fmt/microsoft/files/bmp/pal8w124-out.png");
}

TEST_CASE("Microsoft BMP unflipped images", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/bmp/pal8topdown.bmp",
        "tests/fmt/microsoft/files/bmp/pal8-out.png");
}
