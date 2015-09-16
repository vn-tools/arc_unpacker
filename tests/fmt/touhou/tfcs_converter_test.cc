#include "fmt/touhou/tfcs_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::touhou;

TEST_CASE("Touhou TFCS files", "[fmt]")
{
    TfcsConverter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/touhou/files/tfcs/ItemCommon.csv");
    auto expected_file = tests::file_from_path(
        "tests/fmt/touhou/files/tfcs/ItemCommon-out.csv");
    auto actual_file = converter.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
