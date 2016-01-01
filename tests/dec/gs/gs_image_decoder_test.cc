#include "dec/gs/gs_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::gs;

static const std::string dir = "tests/dec/gs/files/gs-gfx/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = GsImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("GS 8-bit images", "[dec]")
{
    SECTION("8-bit")
    {
        do_test("TRMASK16", "TRMASK16-out.png");
    }

    SECTION("32-bit, opaque")
    {
        do_test("SYSLOGO", "SYSLOGO-out.png");
    }

    SECTION("32-bit, transparent")
    {
        do_test("IMG019KBS", "IMG019KBS-out.png");
    }
}
