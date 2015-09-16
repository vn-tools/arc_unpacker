#include "fmt/kid/cps_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::kid;

TEST_CASE("KID CPS containers", "[fmt]")
{
    CpsConverter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/kid/files/cps/yh04adm.cps");
    auto expected_file = tests::file_from_path(
        "tests/fmt/kid/files/cps/yh04adm.prt");
    auto actual_file = converter.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
