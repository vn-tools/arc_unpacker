#include "formats/kirikiri/tlg_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::Kirikiri;

void test_tlg5_decoding()
{
    TlgConverter converter;
    assert_decoded_image(
        converter,
        L"tests/formats/kirikiri/files/14凛ペンダント.tlg",
        L"tests/formats/kirikiri/files/14凛ペンダント-out.png");
}

void test_tlg6_decoding()
{
    TlgConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/kirikiri/files/tlg6.tlg",
        "tests/formats/kirikiri/files/tlg6-out.png");
}

void test_tlg0_decoding()
{
    TlgConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/kirikiri/files/bg08d.tlg",
        "tests/formats/kirikiri/files/bg08d-out.png");
}

int main(void)
{
    test_tlg5_decoding();
    test_tlg6_decoding();
    test_tlg0_decoding();
    return 0;
}
