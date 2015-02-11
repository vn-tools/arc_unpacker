#include "formats/gfx/tlg_converter.h"
#include "test_support/converter_support.h"

void test_tlg5_decoding()
{
    TlgConverter converter;
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/tlg/14凛ペンダント.tlg",
        "tests/test_files/gfx/tlg/14凛ペンダント-out.png");
}

void test_tlg6_decoding()
{
    TlgConverter converter;
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/tlg/tlg6.tlg",
        "tests/test_files/gfx/tlg/tlg6-out.png");
}

void test_tlg0_decoding()
{
    TlgConverter converter;
    assert_decoded_image(
        converter,
        "tests/test_files/gfx/tlg/bg08d.tlg",
        "tests/test_files/gfx/tlg/bg08d-out.png");
}

int main(void)
{
    test_tlg5_decoding();
    test_tlg6_decoding();
    test_tlg0_decoding();
    return 0;
}
