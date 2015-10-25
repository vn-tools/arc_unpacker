#include "fmt/active_soft/edt_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::active_soft;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    EdtImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_file = tests::image_from_path(expected_path);
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_file);
}

TEST_CASE("ActiveSoft EDT plain images", "[fmt]")
{
    do_test(
        "tests/fmt/active_soft/files/edt/nowprit.edt",
        "tests/fmt/active_soft/files/edt/nowprit-out.png");
}

TEST_CASE("ActiveSoft EDT diff images", "[fmt]")
{
    do_test(
        "tests/fmt/active_soft/files/edt/nowprit.edt",
        "tests/fmt/active_soft/files/edt/nowprit-out.png");
}
