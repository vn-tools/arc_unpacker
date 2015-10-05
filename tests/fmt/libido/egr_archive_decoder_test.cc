#include "fmt/libido/egr_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
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
    auto actual_files = decoder.unpack(*tests::zlib_file_from_path(
        "tests/fmt/libido/files/egr/test-zlib.EGR"));
    REQUIRE(expected_images.size() == actual_files.size());
    for (auto i : util::range(expected_images.size()))
    {
        tests::compare_images(
            *expected_images[i], *tests::image_from_file(*actual_files[i]));
    }
}
