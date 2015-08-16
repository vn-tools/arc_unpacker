#include "fmt/bgi/dsc_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::bgi;

TEST_CASE("Decoding raw DSC files works")
{
    DscConverter converter;
    tests::assert_file_conversion(
        converter,
        "tests/fmt/bgi/files/setupforgallery",
        "tests/fmt/bgi/files/setupforgallery-out.dat");
}

TEST_CASE("Decoding 8-bit DSC images works")
{
    DscConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/bgi/files/SGTitle010000",
        "tests/fmt/bgi/files/SGTitle010000-out.png");
}

TEST_CASE("Decoding 24-bit DSC images works")
{
    DscConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/bgi/files/SGMsgWnd010300",
        "tests/fmt/bgi/files/SGMsgWnd010300-out.png");
}

TEST_CASE("Decoding 32-bit DSC images works")
{
    DscConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/bgi/files/SGTitle000000",
        "tests/fmt/bgi/files/SGTitle000000-out.png");
}
