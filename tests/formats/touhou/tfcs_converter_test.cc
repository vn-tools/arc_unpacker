#include "formats/touhou/tfcs_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"
using namespace Formats::Touhou;

TEST_CASE("Decoding TFCS files works")
{
    TfcsConverter converter;
    assert_decoded_file(
        converter,
        "tests/formats/touhou/files/ItemCommon.csv",
        "tests/formats/touhou/files/ItemCommon-out.csv");
}
