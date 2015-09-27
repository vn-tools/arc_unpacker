#include "fmt/ivory/prs_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::ivory;

TEST_CASE("Ivory PRS images", "[fmt]")
{
    PrsImageDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/ivory/files/prs/BMIK_A16");
    auto expected_image = tests::image_from_path(
        "tests/fmt/ivory/files/prs/BMIK_A16-out.png");
    auto actual_image = decoder.decode(*input_file);
    tests::compare_images(*expected_image, actual_image);
}
