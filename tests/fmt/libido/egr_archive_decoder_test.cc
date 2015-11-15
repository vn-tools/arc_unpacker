#include "fmt/libido/egr_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::libido;

static const std::string dir = "tests/fmt/libido/files/egr/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<pix::Grid>> &expected_images)
{
    const EgrArchiveDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_images(expected_images, actual_files);
}

TEST_CASE("Libido EGR image archives", "[fmt]")
{
    SECTION("Unencrypted")
    {
        do_test(
            "test-zlib.EGR",
            {
                tests::image_from_path(dir + "Image000-out.png"),
                tests::image_from_path(dir + "Image001-out.png"),
            });
    }
}
