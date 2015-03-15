#include "formats/glib/pgx_converter.h"
#include "test_support/converter_support.h"
using namespace Formats::Glib;

void test_pgx_decoding(
    const char *input_image_path, const char *expected_image_path)
{
    PgxConverter converter;
    assert_decoded_image(converter, input_image_path, expected_image_path);
}

int main(void)
{
    //transparent
    test_pgx_decoding(
        "tests/formats/glib/files/CFG_PAGEBTN.PGX",
        "tests/formats/glib/files/CFG_PAGEBTN-out.png");

    //opaque
    test_pgx_decoding(
        "tests/formats/glib/files/BG010A.PGX",
        "tests/formats/glib/files/BG010A-out.png");

    return 0;
}
