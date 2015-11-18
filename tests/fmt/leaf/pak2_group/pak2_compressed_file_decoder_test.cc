#include "fmt/leaf/pak2_group/pak2_compressed_file_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::leaf;

static const std::string dir = "tests/fmt/leaf/files/pak2-compressed-file/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const Pak2CompressedFileDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("Leaf PAK2 compressed files", "[fmt]")
{
    do_test("ttt", "ttt-out");
}
