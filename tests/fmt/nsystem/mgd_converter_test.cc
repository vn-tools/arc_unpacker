#include "fmt/nsystem/mgd_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au::fmt::nsystem;

TEST_CASE("Decoding RGB-based MGD images works")
{
    // possible BGR-RGB issues. Need a non-monochrome sample file.
    MgdConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/fmt/nsystem/files/GS_UD.MGD",
        "tests/fmt/nsystem/files/GS_UD-out.png");
}

TEST_CASE("Decoding PNG-based MGD images works")
{
    MgdConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/fmt/nsystem/files/saveload_p.MGD",
        "tests/fmt/nsystem/files/saveload_p-out.png");
}
