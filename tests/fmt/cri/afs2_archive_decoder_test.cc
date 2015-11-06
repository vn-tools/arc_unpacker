#include "fmt/cri/afs2_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::cri;

TEST_CASE("Cri AFS2 archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("0.dat", "1234567890"_b),
        tests::stub_file("1.dat", "1234567890"_b),
        tests::stub_file("2.dat", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    Afs2ArchiveDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/cri/files/afs2/test.awb");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
