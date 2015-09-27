#include "fmt/alice_soft/vsp_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    VspImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = decoder.decode(*input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Alice Soft VSP images (VSP compression)", "[fmt]")
{
    do_test(
        "tests/fmt/alice_soft/files/vsp/CG8367.VSP",
        "tests/fmt/alice_soft/files/vsp/CG8367-out.png");
}

TEST_CASE("Alice Soft VSP images (PMS compression, monochrome)", "[fmt]")
{
    do_test(
        "tests/fmt/alice_soft/files/vsp/CG_0295.VSP",
        "tests/fmt/alice_soft/files/vsp/CG_0295-out.png");
}

TEST_CASE(
    "Alice Soft VSP images (PMS compression, true color palette)", "[fmt]")
{
    do_test(
        "tests/fmt/alice_soft/files/vsp/045.vsp",
        "tests/fmt/alice_soft/files/vsp/045-out.png");
}
