#include "fmt/microsoft/dds_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::microsoft;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    DdsConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("Decoding DXT1 DDS textures works")
{
    do_test(
        "tests/fmt/microsoft/files/dds/back0.dds",
        "tests/fmt/microsoft/files/dds/back0-out.png");
}

TEST_CASE("Decoding DXT3 DDS textures works")
{
    do_test(
        "tests/fmt/microsoft/files/dds/006_disconnect.dds",
        "tests/fmt/microsoft/files/dds/006_disconnect-out.png");
}

TEST_CASE("Decoding DXT5 DDS textures works")
{
    do_test(
        "tests/fmt/microsoft/files/dds/決1.dds",
        "tests/fmt/microsoft/files/dds/決1-out.png");
}

TEST_CASE("Decoding raw 32-bit DDS textures works")
{
    do_test(
        "tests/fmt/microsoft/files/dds/koishi_7.dds",
        "tests/fmt/microsoft/files/dds/koishi_7-out.png");
}
