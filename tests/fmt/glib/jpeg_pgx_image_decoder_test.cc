#include "fmt/glib/jpeg_pgx_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::glib;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    JpegPgxImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*decoder.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("GLib JPEG PGX opaque images", "[fmt]")
{
    do_test(
        "tests/fmt/glib/files/jpeg_pgx/BG110D.PGX.JPG",
        "tests/fmt/glib/files/jpeg_pgx/BG110D.PGX-out.png");
}
