#include "fmt/cat_system/hg3_image_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::cat_system;

static const std::string dir = "tests/fmt/cat_system/files/hg3/";

static void do_test(
    const std::string input_path,
    const std::vector<std::string> expected_paths)
{
    const Hg3ImageArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    std::vector<std::shared_ptr<io::File>> expected_files;
    for (const auto &path : expected_paths)
        expected_files.push_back(tests::file_from_path(dir + path));
    tests::compare_images(expected_files, actual_files, false);
}

TEST_CASE("CatSystem HG3 images", "[fmt]")
{
    do_test("Tmic27s_00a.hg3", {"Tmic27s_00a-out.png"});
}
