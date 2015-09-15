#include "fmt/sysadv/pga_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::sysadv;

TEST_CASE("Decoding sysadv's PGA images works", "[fmt]")
{
    PgaConverter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/sysadv/files/pga/flower.pga");
    auto expected_image = tests::image_from_path(
        "tests/fmt/sysadv/files/pga/flower-out.png");
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}
