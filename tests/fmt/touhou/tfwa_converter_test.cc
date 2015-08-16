#include "fmt/touhou/tfwa_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::touhou;

TEST_CASE("Decoding TFWA sound files works")
{
    TfwaConverter converter;
    tests::assert_file_conversion(
        converter,
        "tests/fmt/touhou/files/tfwa/2592.wav",
        "tests/fmt/touhou/files/tfwa/2592-out.wav");
}
