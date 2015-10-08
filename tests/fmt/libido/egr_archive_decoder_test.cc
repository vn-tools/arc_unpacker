#include "fmt/libido/egr_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::libido;

TEST_CASE("Libido EGR unencrypted image archives", "[fmt]")
{
    std::vector<std::shared_ptr<pix::Grid>> expected_images
    {
        tests::image_from_path("tests/fmt/libido/files/egr/Image000-out.png"),
        tests::image_from_path("tests/fmt/libido/files/egr/Image001-out.png"),
    };

    EgrArchiveDecoder decoder;
    auto input_file = tests::zlib_file_from_path(
        "tests/fmt/libido/files/egr/test-zlib.EGR");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_images(expected_images, actual_files);
}
