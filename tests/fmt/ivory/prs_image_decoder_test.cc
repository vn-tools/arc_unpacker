#include "fmt/ivory/prs_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::ivory;

TEST_CASE("Ivory PRS images", "[fmt]")
{
    const PrsImageDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/ivory/files/prs/BMIK_A16");
    const auto expected_image = tests::image_from_path(
        "tests/fmt/ivory/files/prs/BMIK_A16-out.png");
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}
