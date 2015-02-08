#include "formats/gfx/spb_converter.h"
#include "test_support/converter_support.h"

void test_spb_decoding(
    const std::string &input_path, const std::string expected_path)
{
    SpbConverter converter;
    assert_decoded_image(converter, input_path, expected_path);
}

int main(void)
{
    test_spb_decoding(
        "tests/test_files/gfx/spb/grimoire_btn.bmp",
        "tests/test_files/gfx/spb/grimoire_btn-out.png");
    return 0;
}
