#include "fmt/twilight_frontier/pak2_image_decoder.h"
#include "io/file_io.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

static void do_test(
    const Pak2ImageDecoder &decoder,
    const std::string &input_path,
    const std::string &expected_path)
{
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_image = tests::image_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const Pak2ImageDecoder decoder;
    do_test(decoder, input_path, expected_path);
}

TEST_CASE("Twilight Frontier CV2 32-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/twilight_frontier/files/pak2/0000_00.cv2",
        "tests/fmt/twilight_frontier/files/pak2/0000_00-out.png");
}

TEST_CASE("Twilight Frontier CV2 24-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/twilight_frontier/files/pak2/0000_00.cv2",
        "tests/fmt/twilight_frontier/files/pak2/0000_00-out.png");
}

TEST_CASE(
    "Twilight Frontier CV2 8-bit images without external palette", "[fmt]")
{
    do_test(
        "tests/fmt/twilight_frontier/files/pak2/stand000.cv2",
        "tests/fmt/twilight_frontier/files/pak2/stand000-out.png");
}

TEST_CASE("Twilight Frontier CV2 8-bit images with external palette", "[fmt]")
{
    Pak2ImageDecoder decoder;

    const auto palette_path
        = "tests/fmt/twilight_frontier/files/pak2/palette000.pal";
    io::FileIO palette_io(palette_path, io::FileMode::Read);
    decoder.add_palette(palette_path, palette_io.read_to_eof());

    do_test(
        decoder,
        "tests/fmt/twilight_frontier/files/pak2/stand000.cv2",
        "tests/fmt/twilight_frontier/files/pak2/stand000-out2.png");
}
