#include "fmt/real_live/ovk_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::real_live;

static const std::string dir = "tests/fmt/real_live/files/ovk/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const OvkArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("RealLive OVK archives", "[fmt]")
{
    do_test(
        "test.ovk",
        {
            tests::stub_file("sample00010", "1234567890"_b),
            tests::stub_file("sample00025", "abcdefghijklmnopqrstuvwxyz"_b),
        });
}
