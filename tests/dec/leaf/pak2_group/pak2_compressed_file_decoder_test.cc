#include "dec/leaf/pak2_group/pak2_compressed_file_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::leaf;

static const std::string dir = "tests/dec/leaf/files/pak2-compressed-file/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = Pak2CompressedFileDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*actual_file, *expected_file, false);
}

TEST_CASE("Leaf PAK2 compressed files", "[dec]")
{
    do_test("ttt", "ttt-out");
}
