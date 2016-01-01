#include "dec/minato_soft/fil_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::minato_soft;

static const std::string dir = "tests/dec/minato_soft/files/fil/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = FilImageDecoder();
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("MinatoSoft FIL mask images", "[dec]")
{
    do_test("Rule07-zlib.fil", "Rule07-out.png");
}
