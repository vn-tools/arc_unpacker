#include "fmt/alice_soft/vsp_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    VspConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("Alice Soft VSP images", "[fmt]")
{
    do_test(
        "tests/fmt/alice_soft/files/vsp/CG8367.VSP",
        "tests/fmt/alice_soft/files/vsp/CG8367-out.png");
}
