#include "fmt/alice_soft/ajp_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "test_support/decoder_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

TEST_CASE("Alice Soft AJP transparent images", "[fmt]")
{
    const AjpImageDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/alice_soft/files/ajp/CG51478.ajp");
    const auto expected_image = tests::image_from_path(
        "tests/fmt/alice_soft/files/ajp/CG51478-out.png");
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}
