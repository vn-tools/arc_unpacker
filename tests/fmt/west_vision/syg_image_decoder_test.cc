#include "fmt/west_vision/syg_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::west_vision;

TEST_CASE("WestVision SYG images", "[fmt]")
{
    SygImageDecoder decoder;
    auto input_file = tests::zlib_file_from_path(
        "tests/fmt/west_vision/files/syg/loadx-zlib.syg");
    auto expected_file = tests::image_from_path(
        "tests/fmt/west_vision/files/syg/loadx-out.png");
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_file);
}
