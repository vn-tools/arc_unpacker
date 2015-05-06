#include "formats/kirikiri/tlg_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"
using namespace Formats::Kirikiri;

TEST_CASE("Decoding TLG5 works")
{
    TlgConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/kirikiri/files/14凛ペンダント.tlg",
        "tests/formats/kirikiri/files/14凛ペンダント-out.png");
}

TEST_CASE("Decoding TLG6 works")
{
    TlgConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/kirikiri/files/tlg6.tlg",
        "tests/formats/kirikiri/files/tlg6-out.png");
}

TEST_CASE("Decoding TLG0 works")
{
    TlgConverter converter;
    assert_decoded_image(
        converter,
        "tests/formats/kirikiri/files/bg08d.tlg",
        "tests/formats/kirikiri/files/bg08d-out.png");
}
