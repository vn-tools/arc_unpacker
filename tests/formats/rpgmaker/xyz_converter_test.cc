#include "formats/rpgmaker/xyz_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::RpgMaker;

void test_xyz_decoding()
{
    XyzConverter converter;
    assert_decoded_image(
        converter,
        L"tests/formats/rpgmaker/files/浅瀬部屋a.xyz",
        L"tests/formats/rpgmaker/files/浅瀬部屋a-out.png");
}

int main(void)
{
    test_xyz_decoding();
    return 0;
}
