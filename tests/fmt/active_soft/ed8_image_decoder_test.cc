#include "fmt/active_soft/ed8_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::active_soft;

TEST_CASE("ActiveSoft ED8 images", "[fmt]")
{
    const Ed8ImageDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/active_soft/files/ed8/EFFECT15.ed8");
    const auto expected_file = tests::image_from_path(
        "tests/fmt/active_soft/files/ed8/EFFECT15-out.png");
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_file);
}
