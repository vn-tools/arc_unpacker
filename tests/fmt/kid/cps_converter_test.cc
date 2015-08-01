#include "fmt/kid/cps_converter.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"

using namespace au::fmt::kid;

TEST_CASE("Decoding CPS containers works")
{
    CpsConverter converter;
    au::tests::assert_decoded_file(
        converter,
        "tests/fmt/kid/files/yh04adm.cps",
        "tests/fmt/kid/files/yh04adm.prt");
}
