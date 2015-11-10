#include "fmt/leaf/leafpack_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::leaf;

TEST_CASE("Leaf LEAFPACK archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    LeafpackArchiveDecoder decoder;
    decoder.set_key("\x51\x42\xFE\x77\x2D\x65\x48\x7E\x0A\x8A\xE5"_b);
    auto input_file = tests::file_from_path(
        "tests/fmt/leaf/files/leafpack/test.pak");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
