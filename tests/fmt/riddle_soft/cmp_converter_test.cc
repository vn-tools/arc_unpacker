#include "fmt/riddle_soft/cmp_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::riddle_soft;

TEST_CASE("Decoding Riddle Soft's CMP files works", "[fmt]")
{
    CmpConverter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/riddle_soft/files/cmp/SLParts.gcp");
    auto expected_file = tests::zlib_file_from_path(
        "tests/fmt/riddle_soft/files/cmp/SLParts-zlib-out.bmp");
    auto actual_file = converter.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
