#include "fmt/kid/prt_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::kid;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    PrtImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = decoder.decode(*input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("KID PRT images", "[fmt]")
{
    do_test(
        "tests/fmt/kid/files/prt/bg01a1.prt",
        "tests/fmt/kid/files/prt/bg01a1-out.png");
}

TEST_CASE("KID PRT images with alpha channel", "[fmt]")
{
    do_test(
        "tests/fmt/kid/files/cps/yh04adm.prt",
        "tests/fmt/kid/files/prt/yh04adm-out.png");
}

TEST_CASE("KID PRT 8-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/kid/files/prt/saver_sm.prt",
        "tests/fmt/kid/files/prt/saver_sm-out.png");
}
