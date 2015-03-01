#include "formats/bgi/cbg_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::Bgi;

void test_cbg_decoding(const char *input_path, const char *expected_path)
{
    CbgConverter converter;
    assert_decoded_image(converter, input_path, expected_path);
}

int main(void)
{
    test_cbg_decoding(
        "tests/formats/bgi/files/3",
        "tests/formats/bgi/files/3-out.png");

    test_cbg_decoding(
        "tests/formats/bgi/files/4",
        "tests/formats/bgi/files/4-out.png");

    test_cbg_decoding(
        "tests/formats/bgi/files/ti_si_de_a1",
        "tests/formats/bgi/files/ti_si_de_a1-out.png");

    return 0;
}
