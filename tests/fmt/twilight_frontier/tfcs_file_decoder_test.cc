#include "fmt/twilight_frontier/tfcs_file_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

static const std::string dir = "tests/fmt/twilight_frontier/files/tfcs/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const TfcsFileDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("Twilight Frontier TFCS files", "[fmt]")
{
    do_test("ItemCommon.csv", "ItemCommon-out.csv");
}
