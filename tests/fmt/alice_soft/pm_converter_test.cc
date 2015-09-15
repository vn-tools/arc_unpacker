#include "fmt/alice_soft/pm_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

TEST_CASE("Decoding PM images works", "[fmt]")
{
    PmConverter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/alice_soft/files/pm/CG40000.pm");
    auto expected_image = tests::image_from_path(
        "tests/fmt/alice_soft/files/pm/CG40000-out.png");
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}
