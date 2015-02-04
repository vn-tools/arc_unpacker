#include "formats/gfx/mgd_converter.h"
#include "test_support/converter_support.h"

void test_mgd_decoding_sgd()
{
    Converter *converter = mgd_converter_create();
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/mgd/GS_UD.MGD",
        "tests/test_files/gfx/mgd/GS_UD-out.png");
    converter_destroy(converter);
}

void test_mgd_decoding_png()
{
    Converter *converter = mgd_converter_create();
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/mgd/saveload_p.MGD",
        "tests/test_files/gfx/mgd/saveload_p-out.png");
    converter_destroy(converter);
}

int main(void)
{
    test_mgd_decoding_sgd();
    test_mgd_decoding_png();
    return 0;
}
