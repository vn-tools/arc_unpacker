#include "fmt/kid/lnd_file_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::kid;

TEST_CASE("KID LND files", "[fmt]")
{
    LndFileDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/kid/files/lnd/CEP037.waf.lnd");
    auto expected_file = tests::file_from_path(
        "tests/fmt/kid/files/waf/CEP037.waf");
    auto actual_file = decoder.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
