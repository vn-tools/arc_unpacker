#include "fmt/fc01/mca_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::fc01;

TEST_CASE("FC01 MCA image archives (unaligned)", "[fmt]")
{
    const std::vector<std::shared_ptr<pix::Grid>> expected_images
    {
        tests::image_from_path("tests/fmt/fc01/files/mca/blin3-out-000.png"),
        tests::image_from_path("tests/fmt/fc01/files/mca/blin3-out-001.png"),
        tests::image_from_path("tests/fmt/fc01/files/mca/blin3-out-002.png"),
        tests::image_from_path("tests/fmt/fc01/files/mca/blin3-out-003.png"),
        tests::image_from_path("tests/fmt/fc01/files/mca/blin3-out-004.png"),
    };

    McaArchiveDecoder decoder;
    decoder.set_key(209);
    const auto input_file = tests::file_from_path(
        "tests/fmt/fc01/files/mca/BLIN3.MCA");
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_images(expected_images, actual_files);
}

TEST_CASE("FC01 MCA image archives (aligned)", "[fmt]")
{
    const std::vector<std::shared_ptr<pix::Grid>> expected_images
    {
        tests::image_from_path("tests/fmt/fc01/files/mca/ok-out-000.png"),
        tests::image_from_path("tests/fmt/fc01/files/mca/ok-out-001.png"),
        tests::image_from_path("tests/fmt/fc01/files/mca/ok-out-002.png"),
        tests::image_from_path("tests/fmt/fc01/files/mca/ok-out-003.png"),
    };

    McaArchiveDecoder decoder;
    decoder.set_key(209);
    const auto input_file = tests::file_from_path(
        "tests/fmt/fc01/files/mca/OK.MCA");
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_images(expected_images, actual_files);
}
