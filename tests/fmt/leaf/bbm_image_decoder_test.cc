#include "fmt/leaf/bbm_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

TEST_CASE("Leaf BBM images", "[fmt]")
{
    auto input_file = tests::zlib_file_from_path(
        "tests/fmt/leaf/files/bbm/Stage_14_E0_OBJ-zlib.bbm");
    auto expected_file = tests::image_from_path(
        "tests/fmt/leaf/files/bbm/Stage_14_E0_OBJ-out.png");
    BbmImageDecoder decoder;
    auto actual_file = decoder.decode(*input_file);
    tests::compare_images(*expected_file, actual_file);
}
