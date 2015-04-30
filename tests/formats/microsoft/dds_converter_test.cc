#include "compat/entry_point.h"
#include "formats/microsoft/dds_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"
using namespace Formats::Microsoft;

static void test(
    const std::string &input_image_path,
    const std::string &expected_image_path)
{
    const std::string prefix("tests/formats/microsoft/files/");
    DdsConverter converter;
    assert_decoded_image(
        converter,
        prefix + input_image_path,
        prefix + expected_image_path);
}

TEST_CASE("Decoding DXT1 DDS textures works")
{
    test("back0.dds", "back0-out.png");
}

TEST_CASE("Decoding DXT3 DDS textures works")
{
    test("006_disconnect.dds", "006_disconnect-out.png");
}

TEST_CASE("Decoding DXT5 DDS textures works")
{
    init_fs_utf8();
    test("決1.dds", "決1-out.png");
}

TEST_CASE("Decoding raw 32-bit DDS textures works")
{
    test("koishi_7.dds", "koishi_7-out.png");
}
