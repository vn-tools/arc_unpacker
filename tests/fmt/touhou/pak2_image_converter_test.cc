#include "io/file_io.h"
#include "fmt/touhou/pak2_image_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::touhou;

TEST_CASE("Decoding CV2 32-bit images works")
{
    Pak2ImageConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/touhou/files/pak2/0000_00.cv2",
        "tests/fmt/touhou/files/pak2/0000_00-out.png");
}

TEST_CASE("Decoding CV2 24-bit images works")
{
    Pak2ImageConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/touhou/files/pak2/0000_00.cv2",
        "tests/fmt/touhou/files/pak2/0000_00-out.png");
}

TEST_CASE("Decoding CV2 8-bit images without external palette works")
{
    Pak2ImageConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/touhou/files/pak2/stand000.cv2",
        "tests/fmt/touhou/files/pak2/stand000-out.png");
}

TEST_CASE("Decoding CV2 8-bit images with external palette works")
{
    Pak2ImageConverter converter;

    const auto palette_path = "tests/fmt/touhou/files/pak2/palette000.pal";
    io::FileIO palette_io(palette_path, io::FileMode::Read);
    converter.add_palette(palette_path, palette_io.read_to_eof());

    auto expected = tests::image_from_converter(
        "tests/fmt/touhou/files/pak2/stand000.cv2", converter);
    auto actual = tests::image_from_path(
        "tests/fmt/touhou/files/pak2/stand000-out2.png");
    tests::compare_images(*expected, *actual);
}
