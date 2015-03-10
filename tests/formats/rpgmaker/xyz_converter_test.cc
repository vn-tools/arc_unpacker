#include "compat/main.h"
#include "formats/rpgmaker/xyz_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::RpgMaker;

void test_xyz_decoding()
{
    XyzConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/rpgmaker/files/浅瀬部屋a.xyz",
        "tests/formats/rpgmaker/files/浅瀬部屋a-out.png");
}

int main(void)
{
    init_fs_utf8();
    test_xyz_decoding();
    return 0;
}
