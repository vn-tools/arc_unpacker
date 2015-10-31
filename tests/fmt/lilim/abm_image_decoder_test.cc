#include "fmt/lilim/abm_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::lilim;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    AbmImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Lilim ABM 32-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/lilim/files/abm/popsave.abm",
        "tests/fmt/lilim/files/abm/popsave-out.png");
}

TEST_CASE("Lilim ABM 8-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/lilim/files/abm/kj_ase.abm",
        "tests/fmt/lilim/files/abm/kj_ase-out.png");
}
