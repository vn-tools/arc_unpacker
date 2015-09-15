#include "fmt/minato_soft/fil_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::minato_soft;

TEST_CASE("Decoding MinatoSoft's FIL mask images works", "[fmt]")
{
    FilConverter converter;
    auto input_file = tests::zlib_file_from_path(
        "tests/fmt/minato_soft/files/fil/Rule07-zlib.fil");
    auto expected_file = tests::image_from_path(
        "tests/fmt/minato_soft/files/fil/Rule07-out.png");
    auto actual_file = tests::image_from_file(*converter.decode(*input_file));
    tests::compare_images(*expected_file, *actual_file, false);
}
