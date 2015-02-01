#include "formats/gfx/cbg_converter.h"
#include "test_support/converter_support.h"

void test_cbg_decoding(const char *input_path, const char *expected_path)
{
    Converter *converter = cbg_converter_create();
    assert_decoded_image(converter, input_path, expected_path);
    converter_destroy(converter);
}

int main(void)
{
    test_cbg_decoding(
        "tests/test_files/gfx/cbg/3",
        "tests/test_files/gfx/cbg/3-out.png");
    test_cbg_decoding(
        "tests/test_files/gfx/cbg/4",
        "tests/test_files/gfx/cbg/4-out.png");
    test_cbg_decoding(
        "tests/test_files/gfx/cbg/ti_si_de_a1",
        "tests/test_files/gfx/cbg/ti_si_de_a1-out.png");
    return 0;
}
