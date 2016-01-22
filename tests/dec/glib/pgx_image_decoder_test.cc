#include "dec/glib/pgx_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::glib;

static const std::string dir = "tests/dec/glib/files/pgx/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = PgxImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("GLib PGX images", "[dec]")
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
