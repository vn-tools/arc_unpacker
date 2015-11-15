#include "fmt/glib/pgx_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::glib;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const PgxImageDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_image = tests::image_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("GLib PGX transparent images", "[fmt]")
{
    do_test(
        "tests/fmt/glib/files/pgx/CFG_PAGEBTN.PGX",
        "tests/fmt/glib/files/pgx/CFG_PAGEBTN-out.png");
}

TEST_CASE("GLib PGX opaque images", "[fmt]")
{
    do_test(
        "tests/fmt/glib/files/pgx/BG010A.PGX",
        "tests/fmt/glib/files/pgx/BG010A-out.png");
}
