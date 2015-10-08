#include "fmt/liar_soft/wcg_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::liar_soft;

TEST_CASE("LiarSoft WCG images", "[fmt]")
{
    WcgImageDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/liar_soft/files/wcg/0003.wcg");
    auto expected_image = tests::image_from_path(
        "tests/fmt/liar_soft/files/wcg/0003-out.png");
    auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}
