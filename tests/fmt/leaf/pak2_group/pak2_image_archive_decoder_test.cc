#include "fmt/leaf/pak2_group/pak2_image_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

static const std::string dir = "tests/fmt/leaf/files/pak2-image/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::string> &expected_paths)
{
    const Pak2ImageArchiveDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    std::vector<std::shared_ptr<io::File>> expected_files;
    for (const auto &path : expected_paths)
        expected_files.push_back(tests::file_from_path(dir + path));
    tests::compare_images(expected_files, actual_files, false);
}

TEST_CASE("Leaf PAK2 images", "[fmt]")
{
    SECTION("Opaque")
    {
        do_test("SpMoji-zlib", {"SpMoji-out.png"});
    }

    SECTION("Masked")
    {
        do_test("c010101b-zlib", {"c010101b-out.png"});
    }
}
