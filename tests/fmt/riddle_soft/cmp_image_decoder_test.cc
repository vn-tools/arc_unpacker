#include "fmt/riddle_soft/cmp_image_decoder.h"
#include "test_support/image_support.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::riddle_soft;

static const std::string dir = "tests/fmt/riddle_soft/files/cmp/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const CmpImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::zlib_file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("RiddleSoft CMP files", "[fmt]")
{
    do_test("SLParts.gcp", "SLParts-zlib-out.bmp");
}
