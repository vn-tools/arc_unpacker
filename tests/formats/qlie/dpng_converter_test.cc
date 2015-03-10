#include "compat/main.h"
#include "formats/qlie/dpng_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::QLiE;

void test_dpng_decoding()
{
    DpngConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/qlie/files/雷02.png",
        "tests/formats/qlie/files/雷02-out.png");
}

int main(void)
{
    init_fs_utf8();
    test_dpng_decoding();
    return 0;
}
