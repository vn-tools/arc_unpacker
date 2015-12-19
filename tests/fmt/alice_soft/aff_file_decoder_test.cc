#include "fmt/alice_soft/aff_file_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const std::string dir = "tests/fmt/alice_soft/files/aff/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const AffFileDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("Alice Soft AFF files", "[fmt]")
{
    do_test("test.aff", "test-out.aff");
}
