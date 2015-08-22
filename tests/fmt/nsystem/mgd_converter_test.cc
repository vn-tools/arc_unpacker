#include "fmt/nsystem/mgd_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::nsystem;

TEST_CASE("Decoding RGB-based MGD images works")
{
    // possible BGR-RGB issues. Need a non-monochrome sample file.
    MgdConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/nsystem/files/mgd/GS_UD.MGD",
        "tests/fmt/nsystem/files/mgd/GS_UD-out.png");
}

TEST_CASE("Decoding PNG-based MGD images works")
{
    MgdConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/nsystem/files/mgd/saveload_p.MGD",
        "tests/fmt/nsystem/files/mgd/saveload_p-out.png");
}
