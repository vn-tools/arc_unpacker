#include "formats/liarsoft/wcg_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::LiarSoft;

void test_wcg_decoding()
{
    WcgConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/liarsoft/files/0003.wcg",
        "tests/formats/liarsoft/files/0003-out.png");
}

int main(void)
{
    test_wcg_decoding();
    return 0;
}
