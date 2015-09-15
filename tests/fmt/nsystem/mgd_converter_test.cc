#include "fmt/nsystem/mgd_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::nsystem;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    MgdConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("Decoding RGB-based MGD images works", "[fmt]")
{
    // possible BGR-RGB issues. Need a non-monochrome sample file.
    do_test(
        "tests/fmt/nsystem/files/mgd/GS_UD.MGD",
        "tests/fmt/nsystem/files/mgd/GS_UD-out.png");
}

TEST_CASE("Decoding PNG-based MGD images works", "[fmt]")
{
    do_test(
        "tests/fmt/nsystem/files/mgd/saveload_p.MGD",
        "tests/fmt/nsystem/files/mgd/saveload_p-out.png");
}
