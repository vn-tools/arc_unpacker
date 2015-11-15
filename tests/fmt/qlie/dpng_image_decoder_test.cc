#include "fmt/qlie/dpng_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::qlie;

TEST_CASE("QLiE DPNG images", "[fmt]")
{
    DpngImageDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/qlie/files/dpng/雷02.png");
    const auto expected_image = tests::image_from_path(
        "tests/fmt/qlie/files/dpng/雷02-out.png");
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}
