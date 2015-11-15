#include "fmt/leaf/bjr_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

TEST_CASE("Leaf BJR images", "[fmt]")
{
    auto input_file = tests::zlib_file_from_path(
        "tests/fmt/leaf/files/bjr/v00232-zlib.BJR");
    input_file->name = "v00232.BJR";
    const auto expected_file = tests::image_from_path(
        "tests/fmt/leaf/files/bjr/v00232-out.png");
    const BjrImageDecoder decoder;
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_file);
}
