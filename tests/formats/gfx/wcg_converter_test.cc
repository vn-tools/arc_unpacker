#include "formats/gfx/wcg_converter.h"
#include "test_support/converter_support.h"

void test_wcg_decoding()
{
    WcgConverter converter;
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/wcg/0003.wcg",
        "tests/test_files/gfx/wcg/0003-out.png");
}

int main(void)
{
    test_wcg_decoding();
    return 0;
}
