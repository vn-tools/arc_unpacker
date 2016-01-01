#include "dec/amuse_craft/pgd_c00_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::amuse_craft;

static const std::string dir = "tests/dec/amuse_craft/files/pgd-c00/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = PgdC00ImageDecoder();
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Amuse Craft PGD images (C00 / TGA)", "[dec]")
{
    do_test("FA03C_01E-zlib.PGD", "FA03C_01E-out.png");
}
