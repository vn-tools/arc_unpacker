#include "fmt/real_live/ovk_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::real_live;

TEST_CASE("RealLive OVK archives", "[fmt]")
{
    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("sample00010", "1234567890"_b),
        tests::stub_file("sample00025", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    const OvkArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/real_live/files/ovk/test.ovk");
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
