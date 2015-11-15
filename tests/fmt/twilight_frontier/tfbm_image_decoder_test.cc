#include "fmt/twilight_frontier/tfbm_image_decoder.h"
#include "io/file_io.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

static const std::string dir = "tests/fmt/twilight_frontier/files/tfbm/";

static void do_test(
    const TfbmImageDecoder &decoder,
    const std::string &input_path,
    const std::string &expected_path)
{
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_image = tests::image_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const TfbmImageDecoder decoder;
    do_test(decoder, input_path, expected_path);
}

TEST_CASE("Twilight Frontier TFBM images", "[fmt]")
{
    SECTION("32-bit")
    {
        do_test("climaxCutA0000.png", "climaxCutA0000-out.png");
    }

    SECTION("16-bit")
    {
        do_test("unk-02479-4461dee8.dat", "unk-02479-4461dee8-out.png");
    }

    SECTION("8-bit, missing external palette")
    {
        do_test("spellB0000.bmp", "spellB0000-out.png");
    }

    SECTION("8-bit, external palette")
    {
        const auto palette_path = dir + "palette000.bmp";
        const auto palette_data
            = tests::file_from_path(palette_path)->io.read_to_eof();

        TfbmImageDecoder decoder;
        decoder.add_palette(palette_path, palette_data);

        do_test(decoder, "spellB0000.bmp", "spellB0000-out2.png");
    }
}
