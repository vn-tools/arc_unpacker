#include "fmt/french_bread/ex3_converter.h"
#include "io/file_io.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"
#include "test_support/image_support.h"
#include "test_support/file_support.h"
#include "util/pack/zlib.h"

using namespace au;
using namespace au::fmt::french_bread;

TEST_CASE("Decoding EX3 images works")
{
    Ex3Converter converter;
    auto compressed_file = tests::file_from_path(
        "tests/fmt/french_bread/files/WIN_HISUI&KOHAKU-out.bmpz");
    auto expected_file = tests::create_file(
        compressed_file->name,
        util::pack::zlib_inflate(compressed_file->io.read_to_eof()));

    auto actual_file = tests::file_from_converter(
        "tests/fmt/french_bread/files/WIN_HISUI&KOHAKU.EX3",
        converter);

    tests::compare_files(*expected_file, *actual_file, false);
}
