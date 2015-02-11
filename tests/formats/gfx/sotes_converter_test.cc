#include "formats/gfx/sotes_converter.h"
#include "test_support/converter_support.h"

void test_sotes_decoding(
    const std::string &input_path, const std::string expected_path)
{
    SotesConverter converter;
    assert_decoded_image(converter, input_path, expected_path);
}

int main(void)
{
    test_sotes_decoding(
        "tests/test_files/gfx/sotes/#1410",
        "tests/test_files/gfx/sotes/#1410-out.png");
    test_sotes_decoding(
        "tests/test_files/gfx/sotes/#1726",
        "tests/test_files/gfx/sotes/#1726-out.png");
    return 0;
}
