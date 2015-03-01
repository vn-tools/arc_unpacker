#include "formats/ivory/prs_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::Ivory;

void test_prs_decoding()
{
    PrsConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/ivory/files/BMIK_A16",
        "tests/formats/ivory/files/BMIK_A16-out.png");
}

int main(void)
{
    test_prs_decoding();
    return 0;
}
