#include "formats/gfx/mgd_converter.h"
#include "test_support/converter_support.h"

void test_mgd_decoding_sgd()
{
    // possible BGR-RGB issues. Need a non-monochrome sample file.
    MgdConverter converter;
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/mgd/GS_UD.MGD",
        "tests/test_files/gfx/mgd/GS_UD-out.png");
}

void test_mgd_decoding_png()
{
    MgdConverter converter;
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/mgd/saveload_p.MGD",
        "tests/test_files/gfx/mgd/saveload_p-out.png");
}

int main(void)
{
    test_mgd_decoding_sgd();
    test_mgd_decoding_png();
    return 0;
}
