#include "fmt/libido/mnc_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::libido;

TEST_CASE("Libido MNC images", "[fmt]")
{
    const MncImageDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(
        "tests/fmt/libido/files/mnc/test-zlib.MNC");
    const auto expected_image = tests::image_from_path(
        "tests/fmt/libido/files/mnc/test-out.png");
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}
