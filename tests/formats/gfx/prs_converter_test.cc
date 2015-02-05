#include "formats/gfx/prs_converter.h"
#include "test_support/converter_support.h"

void test_prs_decoding()
{
    Converter *converter = prs_converter_create();
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/prs/BMIK_A16",
        "tests/test_files/gfx/prs/BMIK_A16-out.png");
    converter_destroy(converter);
}

int main(void)
{
    test_prs_decoding();
    return 0;
}
