#include "fmt/bgi/dsc_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"

using namespace au::fmt;
using namespace au::fmt::bgi;

TEST_CASE("Decoding raw DSC files works")
{
    DscConverter converter;
    au::tests::assert_decoded_file(
        converter,
        "tests/fmt/bgi/files/setupforgallery",
        "tests/fmt/bgi/files/setupforgallery-out.dat");
}

TEST_CASE("Decoding 8-bit DSC images works")
{
    DscConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/fmt/bgi/files/SGTitle010000",
        "tests/fmt/bgi/files/SGTitle010000-out.png");
}

TEST_CASE("Decoding 24-bit DSC images works")
{
    DscConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/fmt/bgi/files/SGMsgWnd010300",
        "tests/fmt/bgi/files/SGMsgWnd010300-out.png");
}

TEST_CASE("Decoding 32-bit DSC images works")
{
    DscConverter converter;
    au::tests::assert_decoded_image(
        converter,
        "tests/fmt/bgi/files/SGTitle000000",
        "tests/fmt/bgi/files/SGTitle000000-out.png");
}
