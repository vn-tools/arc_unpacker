#include "fmt/kirikiri/tlg_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::kirikiri;

TEST_CASE("Decoding TLG5 works")
{
    TlgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/kirikiri/files/tlg5/14凛ペンダント.tlg",
        "tests/fmt/kirikiri/files/tlg5/14凛ペンダント-out.png");
}

TEST_CASE("Decoding TLG6 works")
{
    TlgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/kirikiri/files/tlg6/tlg6.tlg",
        "tests/fmt/kirikiri/files/tlg6/tlg6-out.png");
}

TEST_CASE("Decoding TLG0 works")
{
    TlgConverter converter;
    tests::assert_image_conversion(
        converter,
        "tests/fmt/kirikiri/files/tlg0/bg08d.tlg",
        "tests/fmt/kirikiri/files/tlg0/bg08d-out.png");
}
