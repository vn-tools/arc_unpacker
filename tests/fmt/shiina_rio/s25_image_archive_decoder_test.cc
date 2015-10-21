#include "fmt/shiina_rio/s25_image_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::shiina_rio;

static void do_test(
    const std::string input_path,
    const std::vector<std::string> expected_paths)
{
    S25ImageArchiveDecoder decoder;
    auto input_file = tests::zlib_file_from_path(input_path);
    auto actual_files = tests::unpack(decoder, *input_file);

    std::vector<std::shared_ptr<pix::Grid>> expected_images;
    for (auto &path : expected_paths)
        expected_images.push_back(tests::image_from_path(path));

    tests::compare_images(expected_images, actual_files);
}

TEST_CASE("Shiina Rio S25 images (with dummy offsets)", "[fmt]")
{
    do_test(
        "tests/fmt/shiina_rio/files/s25/BLACK-zlib.S25",
        { "tests/fmt/shiina_rio/files/s25/BLACK-out.png" });
}

TEST_CASE("Shiina Rio S25 images (with empty images)", "[fmt]")
{
    do_test(
        "tests/fmt/shiina_rio/files/s25/BLACK2-zlib.S25",
        { "tests/fmt/shiina_rio/files/s25/BLACK-out.png" });
}

TEST_CASE("Shiina Rio S25 images (alpha channel)", "[fmt]")
{
    do_test(
        "tests/fmt/shiina_rio/files/s25/CURTAIN1-zlib.S25",
        { "tests/fmt/shiina_rio/files/s25/CURTAIN1-out.png" });
}

TEST_CASE("Shiina Rio S25 multi images", "[fmt]")
{
    do_test(
        "tests/fmt/shiina_rio/files/s25/DISPDATE-zlib.S25",
        {
            "tests/fmt/shiina_rio/files/s25/DISPDATE_000-out.png",
            "tests/fmt/shiina_rio/files/s25/DISPDATE_001-out.png",
        });
}
