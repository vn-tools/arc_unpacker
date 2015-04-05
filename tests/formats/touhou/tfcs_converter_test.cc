#include "formats/touhou/tfcs_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::Touhou;

void test_tfcs_decoding()
{
    TfcsConverter converter;
    assert_decoded_file(
        converter,
        "tests/formats/touhou/files/ItemCommon.csv",
        "tests/formats/touhou/files/ItemCommon-out.csv");
}

int main(void)
{
    test_tfcs_decoding();
    return 0;
}
