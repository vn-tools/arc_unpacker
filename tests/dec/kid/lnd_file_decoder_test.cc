#include "dec/kid/lnd_file_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::kid;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = LndFileDecoder();
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_file = tests::file_from_path(expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*actual_file, *expected_file, false);
}

TEST_CASE("KID LND files", "[dec]")
{
    do_test(
        "tests/dec/kid/files/lnd/CEP037.waf.lnd",
        "tests/dec/kid/files/waf/CEP037.waf");
}
