#include "fmt/glib/pgx_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::glib;

static const std::string dir = "tests/fmt/glib/files/pgx/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const PgxImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_image = tests::image_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("GLib PGX images", "[fmt]")
{
    SECTION("Transparent")
    {
        do_test("CFG_PAGEBTN.PGX", "CFG_PAGEBTN-out.png");
    }

    SECTION("Opaque")
    {
        do_test("BG010A.PGX", "BG010A-out.png");
    }
}
