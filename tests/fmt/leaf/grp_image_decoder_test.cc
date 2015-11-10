#include "fmt/leaf/grp_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

TEST_CASE("Leaf GRP images", "[fmt]")
{
    auto palette_file
        = tests::file_from_path("tests/fmt/leaf/files/pak/leaflogo-out.c16");
    auto input_file
        = tests::file_from_path("tests/fmt/leaf/files/pak/leaflogo-out.grp");
    auto expected_file
        = tests::image_from_path("tests/fmt/leaf/files/grp/leaflogo-out.png");

    GrpImageDecoder decoder;
    auto actual_file = decoder.decode(*input_file, *palette_file);
    tests::compare_images(*expected_file, actual_file);
}
