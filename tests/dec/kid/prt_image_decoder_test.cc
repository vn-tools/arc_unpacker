#include "dec/kid/prt_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kid;

static const std::string cps_dir = "tests/dec/kid/files/cps/";
static const std::string prt_dir = "tests/dec/kid/files/prt/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = PrtImageDecoder();
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_file = tests::file_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("KID PRT images", "[dec]")
{
    SECTION("Plain")
    {
        do_test(prt_dir + "bg01a1.prt", prt_dir + "bg01a1-out.png");
    }

    SECTION("Transparency")
    {
        do_test(cps_dir + "yh04adm.prt", prt_dir + "yh04adm-out.png");
    }

    SECTION("8-bit")
    {
        do_test(prt_dir + "saver_sm.prt", prt_dir + "saver_sm-out.png");
    }
}
