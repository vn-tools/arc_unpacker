#include "fmt/touhou/tfbm_converter.h"
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
    TfbmConverter converter;
    do_test(converter, input_path, expected_path);
}

TEST_CASE("Touhou TFBM 32-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/tfbm/climaxCutA0000.png",
        "tests/fmt/touhou/files/tfbm/climaxCutA0000-out.png");
}

TEST_CASE("Touhou TFBM 16-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/tfbm/unk-02479-4461dee8.dat",
        "tests/fmt/touhou/files/tfbm/unk-02479-4461dee8-out.png");
}

TEST_CASE("Touhou TFBM 8-bit images without external palette", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/tfbm/spellB0000.bmp",
        "tests/fmt/touhou/files/tfbm/spellB0000-out.png");
}

TEST_CASE("Touhou TFBM 8-bit images with external palette", "[fmt]")
{
    TfbmConverter converter;

    const auto palette_path = "tests/fmt/touhou/files/tfbm/palette000.bmp";
    io::FileIO palette_io(palette_path, io::FileMode::Read);
    converter.add_palette(palette_path, palette_io.read_to_eof());

    do_test(
        converter,
        "tests/fmt/touhou/files/tfbm/spellB0000.bmp",
        "tests/fmt/touhou/files/tfbm/spellB0000-out2.png");
}
