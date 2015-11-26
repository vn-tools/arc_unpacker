#include "fmt/twilight_frontier/pak2_image_decoder.h"
#include "io/file_stream.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

static const std::string dir = "tests/fmt/twilight_frontier/files/pak2/";

static void do_test(
    const Pak2ImageDecoder &decoder,
    const std::string &input_path,
    const std::string &expected_path)
{
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const Pak2ImageDecoder decoder;
    do_test(decoder, input_path, expected_path);
}

TEST_CASE("Twilight Frontier CV2 images", "[fmt]")
{
    SECTION("32-bit")
    {
        do_test("0000_00.cv2", "0000_00-out.png");
    }

    SECTION("24-bit")
    {
        do_test("0000_00.cv2", "0000_00-out.png");
    }

    SECTION("8-bit, missing external palette")
    {
        do_test("stand000.cv2", "stand000-out.png");
    }

    SECTION("8-bit, external palette")
    {
        const auto palette_path = dir + "palette000.pal";
        const auto palette_data
            = tests::file_from_path(palette_path)->stream.read_to_eof();

        Pak2ImageDecoder decoder;
        decoder.add_palette(palette_path, palette_data);

        do_test(decoder, "stand000.cv2", "stand000-out2.png");
    }
}
