#include "fmt/shiina_rio/ogv_file_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::shiina_rio;

TEST_CASE("Shiina Rio OGV audio", "[fmt]")
{
    OgvFileDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/shiina_rio/files/ogv/TPSE070.OGV");
    auto expected_file = tests::file_from_path(
        "tests/fmt/shiina_rio/files/ogv/TPSE070-out.ogg");
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
