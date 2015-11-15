#include "fmt/sysadv/pga_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::sysadv;

TEST_CASE("sysadv PGA images", "[fmt]")
{
    const PgaImageDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/sysadv/files/pga/flower.pga");
    const auto expected_image = tests::image_from_path(
        "tests/fmt/sysadv/files/pga/flower-out.png");
    const auto actual_file = tests::decode(decoder, *input_file);
    const auto actual_image = tests::image_from_file(*actual_file);
    tests::compare_images(*expected_image, *actual_image);
}
