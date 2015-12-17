#include "fmt/lilim/abm_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::lilim;

static const std::string dir = "tests/fmt/lilim/files/abm/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const AbmImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Lilim ABM images", "[fmt]")
{
    SECTION("32-bit")
    {
        do_test("popsave.abm", "popsave-out.png");
    }

    SECTION("8-bit")
    {
        do_test("kj_ase.abm", "kj_ase-out.png");
    }
}
