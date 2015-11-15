#include "fmt/riddle_soft/cmp_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::riddle_soft;

TEST_CASE("RiddleSoft CMP files", "[fmt]")
{
    const CmpImageDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/riddle_soft/files/cmp/SLParts.gcp");
    const auto expected_file = tests::zlib_file_from_path(
        "tests/fmt/riddle_soft/files/cmp/SLParts-zlib-out.bmp");
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
