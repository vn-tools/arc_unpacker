#include "formats/nsystem/mgd_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::NSystem;

void test_mgd_decoding_sgd()
{
    // possible BGR-RGB issues. Need a non-monochrome sample file.
    MgdConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/nsystem/files/GS_UD.MGD",
        "tests/formats/nsystem/files/GS_UD-out.png");
}

void test_mgd_decoding_png()
{
    MgdConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/nsystem/files/saveload_p.MGD",
        "tests/formats/nsystem/files/saveload_p-out.png");
}

int main(void)
{
    test_mgd_decoding_sgd();
    test_mgd_decoding_png();
    return 0;
}
