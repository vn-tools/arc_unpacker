#include "fmt/alice_soft/ajp_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

TEST_CASE("Alice Soft AJP transparent images", "[fmt]")
{
    AjpImageDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/alice_soft/files/ajp/CG51478.ajp");
    auto expected_image = tests::image_from_path(
        "tests/fmt/alice_soft/files/ajp/CG51478-out.png");
    auto actual_image = decoder.decode(*input_file);
    tests::compare_images(*expected_image, actual_image);
}
