#include "formats/nsystem/mgd_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"

using namespace au::fmt::nsystem;

TEST_CASE("Decoding RGB-based MGD images works")
{
    // possible BGR-RGB issues. Need a non-monochrome sample file.
    MgdConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/formats/nsystem/files/GS_UD.MGD",
        "tests/formats/nsystem/files/GS_UD-out.png");
}

TEST_CASE("Decoding PNG-based MGD images works")
{
    MgdConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/formats/nsystem/files/saveload_p.MGD",
        "tests/formats/nsystem/files/saveload_p-out.png");
}
