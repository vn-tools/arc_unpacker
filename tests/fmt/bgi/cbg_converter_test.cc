#include "fmt/bgi/cbg_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::bgi;

TEST_CASE("Decoding 32-bit CBG1 images works")
{
    CbgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/bgi/files/cbg-v1/ti_si_de_a1",
        "tests/fmt/bgi/files/cbg-v1/ti_si_de_a1-out.png");
}

TEST_CASE("Decoding 24-bit CBG1 images works")
{
    CbgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/bgi/files/cbg-v1/3",
        "tests/fmt/bgi/files/cbg-v1/3-out.png");
}

TEST_CASE("Decoding 8-bit CBG1 images works")
{
    CbgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/bgi/files/cbg-v1/4",
        "tests/fmt/bgi/files/cbg-v1/4-out.png");
}

TEST_CASE("Decoding 32-bit CBG2 images works")
{
    CbgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/bgi/files/cbg-v2/ms_wn_base",
        "tests/fmt/bgi/files/cbg-v2/ms_wn_base-out.png",
        2);
}

TEST_CASE("Decoding 24-bit CBG2 images works")
{
    CbgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/bgi/files/cbg-v2/l_card000",
        "tests/fmt/bgi/files/cbg-v2/l_card000-out.png",
        2);
}

TEST_CASE("Decoding 8-bit CBG2 images works")
{
    CbgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/bgi/files/cbg-v2/mask04r",
        "tests/fmt/bgi/files/cbg-v2/mask04r-out.png",
        2);
}
