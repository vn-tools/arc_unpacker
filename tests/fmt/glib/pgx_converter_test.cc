#include "fmt/glib/pgx_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au::fmt::glib;

static void test_pgx_decoding(
    const std::string &input_image_path,
    const std::string &expected_image_path)
{
    PgxConverter converter;
    au::tests::assert_decoded_image(
        converter, input_image_path, expected_image_path);
}

TEST_CASE("Decoding transparent PGX images works")
{
    test_pgx_decoding(
        "tests/fmt/glib/files/CFG_PAGEBTN.PGX",
        "tests/fmt/glib/files/CFG_PAGEBTN-out.png");
}

TEST_CASE("Decoding opaque PGX images works")
{
    test_pgx_decoding(
        "tests/fmt/glib/files/BG010A.PGX",
        "tests/fmt/glib/files/BG010A-out.png");
}
