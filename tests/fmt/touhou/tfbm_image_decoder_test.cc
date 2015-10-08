#include "fmt/touhou/tfbm_image_decoder.h"
#include "io/file_io.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::touhou;

static void do_test(
    const TfbmImageDecoder &decoder,
    const std::string &input_path,
    const std::string &expected_path)
{
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    TfbmImageDecoder decoder;
    do_test(decoder, input_path, expected_path);
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
    TfbmImageDecoder decoder;

    const auto palette_path = "tests/fmt/touhou/files/tfbm/palette000.bmp";
    io::FileIO palette_io(palette_path, io::FileMode::Read);
    decoder.add_palette(palette_path, palette_io.read_to_eof());

    do_test(
        decoder,
        "tests/fmt/touhou/files/tfbm/spellB0000.bmp",
        "tests/fmt/touhou/files/tfbm/spellB0000-out2.png");
}
