#include "dec/playstation/gxt_image_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::playstation;

static const std::string dir = "tests/dec/playstation/files/gxt/";

static void do_test(
    const std::string input_path,
    const std::vector<std::string> expected_paths)
{
    const auto decoder = GxtImageArchiveDecoder();
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    std::vector<std::shared_ptr<io::File>> expected_files;
    for (const auto &path : expected_paths)
        expected_files.push_back(tests::file_from_path(dir + path));
    tests::compare_images(expected_files, actual_files, false);
}

TEST_CASE("Playstation GXT images", "[dec]")
{
    do_test("mask15-zlib.gxt", {"mask15-out.png"});
}
