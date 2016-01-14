#include "dec/playstation/gtf_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::playstation;

static const std::string dir = "tests/dec/playstation/files/gtf/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = GtfImageDecoder();
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Playstation GTF images", "[dec]")
{
    SECTION("DXT5")
    {
        do_test("DATA03-zlib.GTF", "DATA03-out.png");
    }
}
