#include "fmt/glib/pgx_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::glib;

TEST_CASE("Decoding transparent PGX images works")
{
    PgxConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/glib/files/pgx/CFG_PAGEBTN.PGX",
        "tests/fmt/glib/files/pgx/CFG_PAGEBTN-out.png");
}

TEST_CASE("Decoding opaque PGX images works")
{
    PgxConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/glib/files/pgx/BG010A.PGX",
        "tests/fmt/glib/files/pgx/BG010A-out.png");
}
