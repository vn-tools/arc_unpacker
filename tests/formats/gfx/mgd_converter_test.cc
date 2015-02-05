#include "formats/gfx/mgd_converter.h"
#include "test_support/converter_support.h"

void test_mgd_decoding_sgd()
{
    Converter *converter = new MgdConverter();
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/mgd/GS_UD.MGD",
        "tests/test_files/gfx/mgd/GS_UD-out.png");
    delete converter;
}

void test_mgd_decoding_png()
{
    Converter *converter = new MgdConverter();
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/mgd/saveload_p.MGD",
        "tests/test_files/gfx/mgd/saveload_p-out.png");
    delete converter;
}

int main(void)
{
    test_mgd_decoding_sgd();
    test_mgd_decoding_png();
    return 0;
}
