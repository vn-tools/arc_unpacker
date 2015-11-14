#include "fmt/leaf/pak2_image_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

static void do_test(
    const std::string input_path,
    const std::vector<std::string> expected_paths)
{
    const Pak2ImageArchiveDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);

    std::vector<std::shared_ptr<pix::Grid>> expected_images;
    for (const auto &path : expected_paths)
        expected_images.push_back(tests::image_from_path(path));

    tests::compare_images(expected_images, actual_files);
}

TEST_CASE("Leaf PAK2 images", "[fmt]")
{
    do_test(
        "tests/fmt/leaf/files/pak2-image/SpMoji-zlib",
        { "tests/fmt/leaf/files/pak2-image/SpMoji-out.png" });
}

TEST_CASE("Leaf PAK2 images (with transparency mask)", "[fmt]")
{
    do_test(
        "tests/fmt/leaf/files/pak2-image/c010101b-zlib",
        { "tests/fmt/leaf/files/pak2-image/c010101b-out.png" });
}
