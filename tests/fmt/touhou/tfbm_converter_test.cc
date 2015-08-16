#include "io/file_io.h"
#include "fmt/touhou/tfbm_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::touhou;

TEST_CASE("Decoding 32-bit TFBM images works")
{
    TfbmConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/touhou/files/tfbm/climaxCutA0000.png",
        "tests/fmt/touhou/files/tfbm/climaxCutA0000-out.png");
}

TEST_CASE("Decoding 16-bit TFBM images works")
{
    TfbmConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/touhou/files/tfbm/unk-02479-4461dee8.dat",
        "tests/fmt/touhou/files/tfbm/unk-02479-4461dee8-out.png");
}

TEST_CASE("Decoding 8-bit TFBM images without external palette works")
{
    TfbmConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/touhou/files/tfbm/spellB0000.bmp",
        "tests/fmt/touhou/files/tfbm/spellB0000-out.png");
}

TEST_CASE("Decoding 8-bit TFBM images with external palette works")
{
    TfbmConverter converter;

    const auto palette_path = "tests/fmt/touhou/files/tfbm/palette000.bmp";
    io::FileIO palette_io(palette_path, io::FileMode::Read);
    converter.add_palette(palette_path, palette_io.read_to_eof());

    auto expected = tests::image_from_converter(
        "tests/fmt/touhou/files/tfbm/spellB0000.bmp", converter);
    auto actual = tests::image_from_path(
        "tests/fmt/touhou/files/tfbm/spellB0000-out2.png");
    tests::compare_images(*expected, *actual);
}
