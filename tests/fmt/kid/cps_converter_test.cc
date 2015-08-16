#include "fmt/kid/cps_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::kid;

TEST_CASE("Decoding CPS containers works")
{
    CpsConverter converter;
    tests::assert_file_conversion(
        converter,
        "tests/fmt/kid/files/yh04adm.cps",
        "tests/fmt/kid/files/yh04adm.prt");
}
