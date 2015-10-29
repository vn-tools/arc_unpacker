#include "fmt/lilim/scr_file_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::lilim;

TEST_CASE("Lilim AOS scripts", "[fmt]")
{
    ScrFileDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/lilim/files/scr/var.scr");
    auto expected_file = tests::file_from_path(
        "tests/fmt/lilim/files/scr/var-out.txt");
    expected_file->name = "tests/fmt/lilim/files/scr/var.txt";
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, true);
}
