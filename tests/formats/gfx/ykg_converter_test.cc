#include "formats/gfx/ykg_converter.h"
#include "test_support/converter_support.h"

void test_ykg_decoding()
{
    YkgConverter converter;
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/ykg/reimu.ykg",
        "tests/test_files/gfx/ykg/reimu-out.png");
}

int main(void)
{
    test_ykg_decoding();
    return 0;
}

