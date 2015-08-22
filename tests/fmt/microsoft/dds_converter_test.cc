#include "fmt/microsoft/dds_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::microsoft;

TEST_CASE("Decoding DXT1 DDS textures works")
{
    DdsConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/microsoft/files/dds/back0.dds",
        "tests/fmt/microsoft/files/dds/back0-out.png");
}

TEST_CASE("Decoding DXT3 DDS textures works")
{
    DdsConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/microsoft/files/dds/006_disconnect.dds",
        "tests/fmt/microsoft/files/dds/006_disconnect-out.png");
}

TEST_CASE("Decoding DXT5 DDS textures works")
{
    DdsConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/microsoft/files/dds/決1.dds",
        "tests/fmt/microsoft/files/dds/決1-out.png");
}

TEST_CASE("Decoding raw 32-bit DDS textures works")
{
    DdsConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/microsoft/files/dds/koishi_7.dds",
        "tests/fmt/microsoft/files/dds/koishi_7-out.png");
}
