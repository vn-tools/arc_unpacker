#include "fmt/lilim/dbm_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::lilim;

static const std::string dir = "tests/fmt/lilim/files/dbm/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const DbmImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_image = tests::image_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Lilim DBM images", "[fmt]")
{
    SECTION("Format 1")
    {
        do_test("C01.dbm", "C01-out.png");
    }

    SECTION("Format 2")
    {
        do_test("prompt.dbm", "prompt-out.png");
    }

    SECTION("Format 3")
    {
        do_test("WD04.dbm", "WD04-out.png");
    }

    SECTION("Format 4")
    {
        do_test("TB_fin.dbm", "TB_fin-out.png");
    }
}
