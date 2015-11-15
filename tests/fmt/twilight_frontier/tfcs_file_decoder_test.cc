#include "fmt/twilight_frontier/tfcs_file_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

TEST_CASE("Twilight Frontier TFCS files", "[fmt]")
{
    const TfcsFileDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/twilight_frontier/files/tfcs/ItemCommon.csv");
    const auto expected_file = tests::file_from_path(
        "tests/fmt/twilight_frontier/files/tfcs/ItemCommon-out.csv");
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
