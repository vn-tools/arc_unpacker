#include "fmt/bgi/dsc_file_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::bgi;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    DscFileDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_file = tests::decode(decoder, *input_file);
    auto actual_image = tests::image_from_file(*actual_file);
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("BGI DSC raw files", "[fmt]")
{
    DscFileDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/bgi/files/dsc/setupforgallery");
    auto expected_file = tests::file_from_path(
        "tests/fmt/bgi/files/dsc/setupforgallery-out.dat");
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("BGI DSC 8-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/bgi/files/dsc/SGTitle010000",
        "tests/fmt/bgi/files/dsc/SGTitle010000-out.png");
}

TEST_CASE("BGI DSC 24-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/bgi/files/dsc/SGMsgWnd010300",
        "tests/fmt/bgi/files/dsc/SGMsgWnd010300-out.png");
}

TEST_CASE("BGI DSC 32-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/bgi/files/dsc/SGTitle000000",
        "tests/fmt/bgi/files/dsc/SGTitle000000-out.png");
}
