#include "formats/lizsoft/sotes_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::Lizsoft;

void test_sotes_decoding(
    const std::string &input_path, const std::string expected_path)
{
    SotesConverter converter;
    assert_decoded_image(converter, input_path, expected_path);
}

int main(void)
{
    test_sotes_decoding(
        "tests/formats/lizsoft/files/#1410",
        "tests/formats/lizsoft/files/#1410-out.png");

    test_sotes_decoding(
        "tests/formats/lizsoft/files/#1726",
        "tests/formats/lizsoft/files/#1726-out.png");
    return 0;
}
