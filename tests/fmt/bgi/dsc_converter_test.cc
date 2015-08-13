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
