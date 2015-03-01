#include "formats/key/g00_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::Key;

void test_g00_decoding(
    const char *input_image_path, const char *expected_image_path)
{
    G00Converter converter;
    assert_decoded_image(converter, input_image_path, expected_image_path);
}

int main(void)
{
    //v0
    test_g00_decoding(
        "tests/formats/key/files/ayu_02.g00",
        "tests/formats/key/files/ayu_02-out.png");

    //v2
    test_g00_decoding(
        "tests/formats/key/files/AYU_03.g00",
        "tests/formats/key/files/AYU_03-out.png");

    //v1
    test_g00_decoding(
        "tests/formats/key/files/ayu_05.g00",
        "tests/formats/key/files/ayu_05-out.png");

    return 0;
}
