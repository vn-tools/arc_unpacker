#include "dec/leaf/kcap_group/bbm_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::leaf;

static const std::string dir = "tests/dec/leaf/files/bbm/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = BbmImageDecoder();
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Leaf BBM images", "[dec]")
{
    do_test("Stage_14_E0_OBJ-zlib.bbm", "Stage_14_E0_OBJ-out.png");
}
