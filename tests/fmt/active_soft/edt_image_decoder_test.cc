#include "fmt/active_soft/edt_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::active_soft;

TEST_CASE("ActiveSoft EDT images", "[fmt]")
{
    EdtImageDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/active_soft/files/edt/nowprit.edt");
    auto expected_file = tests::image_from_path(
        "tests/fmt/active_soft/files/edt/nowprit-out.png");
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_file);
}
