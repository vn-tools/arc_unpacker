#include "fmt/nscripter/spb_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::nscripter;

TEST_CASE("NScripter SPB-compressed stream", "[fmt]")
{
    SpbImageDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/nscripter/files/spb/grimoire_btn.bmp");
    auto expected_image = tests::image_from_path(
        "tests/fmt/nscripter/files/spb/grimoire_btn-out.png");
    auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}
