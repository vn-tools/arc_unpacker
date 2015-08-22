#include "fmt/kid/prt_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::kid;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    PrtConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("Decoding PRT images works")
{
    do_test(
        "tests/fmt/kid/files/prt/bg01a1.prt",
        "tests/fmt/kid/files/prt/bg01a1-out.png");
}

TEST_CASE("Decoding PRT images with alpha channel works")
{
    do_test(
        "tests/fmt/kid/files/cps/yh04adm.prt",
        "tests/fmt/kid/files/prt/yh04adm-out.png");
}

TEST_CASE("Decoding 8-bit PRT images works")
{
    do_test(
        "tests/fmt/kid/files/prt/saver_sm.prt",
        "tests/fmt/kid/files/prt/saver_sm-out.png");
}
