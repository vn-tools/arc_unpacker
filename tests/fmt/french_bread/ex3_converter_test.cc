#include "fmt/french_bread/ex3_converter.h"
#include "io/file_io.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "util/pack/zlib.h"

using namespace au;
using namespace au::fmt::french_bread;

TEST_CASE("Decoding EX3 images works", "[fmt]")
{
    Ex3Converter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/french_bread/files/ex3/WIN_HISUI&KOHAKU.EX3");
    auto expected_file = tests::zlib_file_from_path(
        "tests/fmt/french_bread/files/ex3/WIN_HISUI&KOHAKU-zlib-out.bmp");
    auto actual_file = converter.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
