#include "fmt/french_bread/ex3_image_decoder.h"
#include "io/file_io.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "util/pack/zlib.h"

using namespace au;
using namespace au::fmt::french_bread;

TEST_CASE("FrenchBread EX3 images", "[fmt]")
{
    Ex3ImageDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/french_bread/files/ex3/WIN_HISUI&KOHAKU.EX3");
    auto expected_file = tests::zlib_file_from_path(
        "tests/fmt/french_bread/files/ex3/WIN_HISUI&KOHAKU-zlib-out.bmp");
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
