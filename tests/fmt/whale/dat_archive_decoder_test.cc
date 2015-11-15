#include "fmt/whale/dat_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::whale;

static void do_test(DatArchiveDecoder &decoder, const std::string &path)
{
    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    const auto input_file = tests::file_from_path(path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Whale DAT plain archives", "[fmt]")
{
    DatArchiveDecoder decoder;
    decoder.add_file_name("123.txt");
    decoder.add_file_name("abc.txt");
    do_test(decoder, "tests/fmt/whale/files/dat/plain.dat");
}

TEST_CASE("Whale DAT compressed archives", "[fmt]")
{
    DatArchiveDecoder decoder;
    decoder.add_file_name("123.txt");
    decoder.add_file_name("abc.txt");
    decoder.set_game_title("A Dog Story");
    do_test(decoder, "tests/fmt/whale/files/dat/compressed.dat");
}

TEST_CASE("Whale DAT compressed archives with unknown names", "[fmt]")
{
    DatArchiveDecoder decoder;
    decoder.set_game_title("A Dog Story");

    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("0000.txt", "1234567890"_b),
        tests::stub_file("0001.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    const auto input_file = tests::file_from_path(
        "tests/fmt/whale/files/dat/compressed.dat");
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Whale DAT fully encrypted archives", "[fmt]")
{
    DatArchiveDecoder decoder;
    decoder.add_file_name("123.txt");
    decoder.add_file_name("abc.txt");
    do_test(decoder, "tests/fmt/whale/files/dat/encrypted.dat");
}
