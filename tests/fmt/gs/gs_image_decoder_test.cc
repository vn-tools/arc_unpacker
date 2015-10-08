#include "fmt/gs/gs_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::gs;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    GsImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("GS 8-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/gs/files/gs-gfx/TRMASK16",
        "tests/fmt/gs/files/gs-gfx/TRMASK16-out.png");
}

TEST_CASE("GS 32-bit opaque images", "[fmt]")
{
    do_test(
        "tests/fmt/gs/files/gs-gfx/SYSLOGO",
        "tests/fmt/gs/files/gs-gfx/SYSLOGO-out.png");
}

TEST_CASE("GS 32-bit transparent images", "[fmt]")
{
    do_test(
        "tests/fmt/gs/files/gs-gfx/IMG019KBS",
        "tests/fmt/gs/files/gs-gfx/IMG019KBS-out.png");
}
