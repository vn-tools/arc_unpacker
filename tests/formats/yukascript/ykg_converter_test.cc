#include "formats/yukascript/ykg_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::YukaScript;

void test_ykg_decoding()
{
    YkgConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/yukascript/files/reimu.ykg",
        "tests/formats/yukascript/files/reimu-out.png");
}

int main(void)
{
    test_ykg_decoding();
    return 0;
}

