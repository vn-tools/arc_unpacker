#include "fmt/leaf/lfg_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    LfgImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_file = tests::image_from_path(expected_path);
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_file);
}

TEST_CASE("Leaf LFG audio (vertical coding)", "[fmt]")
{
    do_test(
        "tests/fmt/leaf/files/lfg/OP_L6.LFG",
        "tests/fmt/leaf/files/lfg/OP_L6-out.png");
}

TEST_CASE("Leaf LFG audio (horizontal coding)", "[fmt]")
{
    do_test(
        "tests/fmt/leaf/files/lfg/MAX_C09.LFG",
        "tests/fmt/leaf/files/lfg/MAX_C09-out.png");
}
