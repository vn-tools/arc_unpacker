#include "formats/gfx/dpng_converter.h"
#include "test_support/converter_support.h"

void test_dpng_decoding()
{
    DpngConverter converter;
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/dpng/雷02.png",
        "tests/test_files/gfx/dpng/雷02-out.png");
}

int main(void)
{
    test_dpng_decoding();
    return 0;
}
