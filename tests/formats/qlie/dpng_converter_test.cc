#include "formats/qlie/dpng_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::QLiE;

void test_dpng_decoding()
{
    DpngConverter converter;
    assert_decoded_image(
        converter,
        L"tests/formats/qlie/files/雷02.png",
        L"tests/formats/qlie/files/雷02-out.png");
}

int main(void)
{
    test_dpng_decoding();
    return 0;
}
