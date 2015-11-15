#include "fmt/lilim/dbm_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::lilim;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const DbmImageDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_image = tests::image_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Lilim DBM images (format 1)", "[fmt]")
{
    do_test(
        "tests/fmt/lilim/files/dbm/C01.dbm",
        "tests/fmt/lilim/files/dbm/C01-out.png");
}

TEST_CASE("Lilim DBM images (format 2)", "[fmt]")
{
    do_test(
        "tests/fmt/lilim/files/dbm/prompt.dbm",
        "tests/fmt/lilim/files/dbm/prompt-out.png");
}

TEST_CASE("Lilim DBM images (format 3)", "[fmt]")
{
    do_test(
        "tests/fmt/lilim/files/dbm/WD04.dbm",
        "tests/fmt/lilim/files/dbm/WD04-out.png");
}

TEST_CASE("Lilim DBM images (format 4)", "[fmt]")
{
    do_test(
        "tests/fmt/lilim/files/dbm/TB_fin.dbm",
        "tests/fmt/lilim/files/dbm/TB_fin-out.png");
}
