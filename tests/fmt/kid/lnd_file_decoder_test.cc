#include "fmt/kid/lnd_file_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::kid;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const LndFileDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_file = tests::file_from_path(expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("KID LND files", "[fmt]")
{
    do_test(
        "tests/fmt/kid/files/lnd/CEP037.waf.lnd",
        "tests/fmt/kid/files/waf/CEP037.waf");
}
