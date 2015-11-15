#include "fmt/shiina_rio/s25_image_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::shiina_rio;

static const std::string dir = "tests/fmt/shiina_rio/files/s25/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::string> &expected_paths)
{
    const S25ImageArchiveDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    std::vector<std::shared_ptr<pix::Grid>> expected_images;
    for (auto &path : expected_paths)
        expected_images.push_back(tests::image_from_path(dir + path));
    tests::compare_images(expected_images, actual_files);
}

TEST_CASE("Shiina Rio S25 images", "[fmt]")
{
    SECTION("Dummy offsets")
    {
        do_test("BLACK-zlib.S25", {"BLACK-out.png"});
    }

    SECTION("Empty images")
    {
        do_test("BLACK2-zlib.S25", {"BLACK-out.png"});
    }

    SECTION("Alpha channel")
    {
        do_test("CURTAIN1-zlib.S25", {"CURTAIN1-out.png"});
    }

    SECTION("Multi images")
    {
        do_test(
            "DISPDATE-zlib.S25",
            {"DISPDATE_000-out.png", "DISPDATE_001-out.png"});
    }
}
