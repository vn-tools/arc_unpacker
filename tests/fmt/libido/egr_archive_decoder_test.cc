#include "fmt/libido/egr_archive_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::libido;

static const std::string dir = "tests/fmt/libido/files/egr/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const EgrArchiveDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_images(expected_files, actual_files, false);
}

TEST_CASE("Libido EGR image archives", "[fmt]")
{
    SECTION("Unencrypted")
    {
        do_test(
            "test-zlib.EGR",
            {
                tests::file_from_path(dir + "Image000-out.png"),
                tests::file_from_path(dir + "Image001-out.png"),
            });
    }
}
