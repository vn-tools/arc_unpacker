#include "fmt/nsystem/mgd_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::nsystem;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    MgdImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("NSystem MGD RGB-based images", "[fmt]")
{
    // possible BGR-RGB issues. Need a non-monochrome sample file.
    do_test(
        "tests/fmt/nsystem/files/mgd/GS_UD.MGD",
        "tests/fmt/nsystem/files/mgd/GS_UD-out.png");
}

TEST_CASE("NSystem MGD PNG-based images", "[fmt]")
{
    do_test(
        "tests/fmt/nsystem/files/mgd/saveload_p.MGD",
        "tests/fmt/nsystem/files/mgd/saveload_p-out.png");
}
