#include "fmt/microsoft/dds_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::microsoft;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    DdsImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Microsoft DDS DXT1 textures", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/dds/back0.dds",
        "tests/fmt/microsoft/files/dds/back0-out.png");
}

TEST_CASE("Microsoft DDS DXT3 textures", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/dds/006_disconnect.dds",
        "tests/fmt/microsoft/files/dds/006_disconnect-out.png");
}

TEST_CASE("Microsoft DDS DXT5 textures", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/dds/決1.dds",
        "tests/fmt/microsoft/files/dds/決1-out.png");
}

TEST_CASE("Microsoft DDS raw 32-bit textures", "[fmt]")
{
    do_test(
        "tests/fmt/microsoft/files/dds/koishi_7.dds",
        "tests/fmt/microsoft/files/dds/koishi_7-out.png");
}
