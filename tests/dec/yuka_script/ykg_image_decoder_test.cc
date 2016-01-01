#include "dec/yuka_script/ykg_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::yuka_script;

static const std::string dir = "tests/dec/yuka_script/files/ykg/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = YkgImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("YukaScript YKG images", "[dec]")
{
    do_test("reimu.ykg", "reimu-out.png");
}
