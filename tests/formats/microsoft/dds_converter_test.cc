#include "compat/main.h"
#include "formats/microsoft/dds_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::Microsoft;

void test(const std::string &input_path, const std::string &expected_path)
{
    const std::string prefix("tests/formats/microsoft/files/");
    DdsConverter converter;
    assert_decoded_image(
        converter, prefix + input_path, prefix + expected_path);
}

int main(void)
{
    init_fs_utf8();

    //dxt1
    test("back0.dds", "back0-out.png");

    //dxt3
    test("006_disconnect.dds", "006_disconnect-out.png");

    //dxt5
    test("決1.dds", "決1-out.png");

    //raw 32-bit
    test("koishi_7.dds", "koishi_7-out.png");

    return 0;
}
