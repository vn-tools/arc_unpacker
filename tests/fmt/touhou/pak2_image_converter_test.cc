#include "fmt/touhou/pak2_image_converter.h"
#include "io/file_io.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::touhou;

static void do_test(
    const fmt::Converter &converter,
    const std::string &input_path,
    const std::string &expected_path)
{
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    Pak2ImageConverter converter;
    do_test(converter, input_path, expected_path);
}

TEST_CASE("Decoding CV2 32-bit images works")
{
    do_test(
        "tests/fmt/touhou/files/pak2/0000_00.cv2",
        "tests/fmt/touhou/files/pak2/0000_00-out.png");
}

TEST_CASE("Decoding CV2 24-bit images works")
{
    do_test(
        "tests/fmt/touhou/files/pak2/0000_00.cv2",
        "tests/fmt/touhou/files/pak2/0000_00-out.png");
}

TEST_CASE("Decoding CV2 8-bit images without external palette works")
{
    do_test(
        "tests/fmt/touhou/files/pak2/stand000.cv2",
        "tests/fmt/touhou/files/pak2/stand000-out.png");
}

TEST_CASE("Decoding CV2 8-bit images with external palette works")
{
    Pak2ImageConverter converter;

    const auto palette_path = "tests/fmt/touhou/files/pak2/palette000.pal";
    io::FileIO palette_io(palette_path, io::FileMode::Read);
    converter.add_palette(palette_path, palette_io.read_to_eof());

    do_test(
        converter,
        "tests/fmt/touhou/files/pak2/stand000.cv2",
        "tests/fmt/touhou/files/pak2/stand000-out2.png");
}
