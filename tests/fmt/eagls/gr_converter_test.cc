#include "fmt/eagls/gr_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::eagls;

TEST_CASE("Decoding EAGLS's GR images works")
{
    GrConverter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/eagls/files/gr/mask17.gr");
    auto expected_file = tests::zlib_file_from_path(
        "tests/fmt/eagls/files/gr/mask17-zlib-out.bmp");
    auto actual_file = converter.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
