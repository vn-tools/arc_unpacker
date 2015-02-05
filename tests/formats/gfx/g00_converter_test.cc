#include "formats/gfx/g00_converter.h"
#include "test_support/converter_support.h"

void test_g00_decoding(
    const char *input_image_path,
    const char *expected_image_path)
{
    Converter *converter = g00_converter_create();
    assert_decoded_image(
        converter,
        input_image_path,
        expected_image_path);
    converter_destroy(converter);
}

int main(void)
{
    //v0
    test_g00_decoding(
        "tests/test_files/gfx/g00/ayu_02.g00",
        "tests/test_files/gfx/g00/ayu_02-out.png");

    //v2
    test_g00_decoding(
        "tests/test_files/gfx/g00/AYU_03.g00",
        "tests/test_files/gfx/g00/AYU_03-out.png");

    //v1
    test_g00_decoding(
        "tests/test_files/gfx/g00/ayu_05.g00",
        "tests/test_files/gfx/g00/ayu_05-out.png");

    return 0;
}
