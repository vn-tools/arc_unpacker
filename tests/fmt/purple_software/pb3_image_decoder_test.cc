#include "fmt/purple_software/pb3_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::purple_software;

static const std::string dir = "tests/fmt/purple_software/files/pb3/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const Pb3ImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Purple Software PB3 images", "[fmt]")
{
    SECTION("Version 1")
    {
        do_test("dialog_saki.pb3", "dialog_saki-out.png");
    }
}
