#include "fmt/fc01/mca_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::fc01;

static const std::string dir = "tests/fmt/fc01/files/mca/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const u8 key)
{
    McaArchiveDecoder decoder;
    decoder.set_key(key);
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_images(expected_files, actual_files, false);
}

TEST_CASE("FC01 MCA image archives", "[fmt]")
{
    SECTION("Unaligned")
    {
        do_test(
            "BLIN3.MCA",
            {
                tests::file_from_path(dir + "blin3-out-000.png"),
                tests::file_from_path(dir + "blin3-out-001.png"),
                tests::file_from_path(dir + "blin3-out-002.png"),
                tests::file_from_path(dir + "blin3-out-003.png"),
                tests::file_from_path(dir + "blin3-out-004.png"),
            },
            209);
    }

    SECTION("Aligned")
    {
        do_test(
            "OK.MCA",
            {
                tests::file_from_path(dir + "ok-out-000.png"),
                tests::file_from_path(dir + "ok-out-001.png"),
                tests::file_from_path(dir + "ok-out-002.png"),
                tests::file_from_path(dir + "ok-out-003.png"),
            },
            209);
    }
}
