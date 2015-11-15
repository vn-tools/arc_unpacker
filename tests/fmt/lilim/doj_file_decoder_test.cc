#include "fmt/lilim/doj_file_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::lilim;

TEST_CASE("Lilim DOJ scripts", "[fmt]")
{
    const DojFileDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/lilim/files/doj/trueblue.doj");
    const auto expected_file = tests::file_from_path(
        "tests/fmt/lilim/files/doj/trueblue-out.doj");
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
