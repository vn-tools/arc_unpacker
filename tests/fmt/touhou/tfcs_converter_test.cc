#include "fmt/touhou/tfcs_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::touhou;

TEST_CASE("Decoding TFCS files works")
{
    TfcsConverter converter;
    tests::assert_file_conversion(
        converter,
        "tests/fmt/touhou/files/ItemCommon.csv",
        "tests/fmt/touhou/files/ItemCommon-out.csv");
}
