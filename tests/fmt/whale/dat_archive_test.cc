#include "fmt/whale/dat_archive.h"
#include "test_support/archive_support.h"
#include "test_support/file_support.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt::whale;

static void do_test(DatArchive &archive, const std::string &path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    auto actual_files = au::tests::unpack_to_memory(path, archive);

    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Unpacking plain Whale's DAT archives works", "[fmt]")
{
    DatArchive archive;
    archive.add_file_name("123.txt");
    archive.add_file_name("abc.txt");
    do_test(archive, "tests/fmt/whale/files/dat/plain.dat");
}

TEST_CASE("Unpacking compressed Whale's DAT archives works", "[fmt]")
{
    DatArchive archive;
    archive.add_file_name("123.txt");
    archive.add_file_name("abc.txt");
    archive.set_game_title("A Dog Story");
    do_test(archive, "tests/fmt/whale/files/dat/compressed.dat");
}

TEST_CASE(
    "Unpacking compressed Whale's DAT archives with unknown names works",
    "[fmt]")
{
    DatArchive archive;
    archive.set_game_title("A Dog Story");

    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("0000.txt", "1234567890"_b),
        tests::stub_file("0001.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    auto actual_files = au::tests::unpack_to_memory(
        "tests/fmt/whale/files/dat/compressed.dat", archive);

    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Unpacking fully encrypted Whale's DAT archives works", "[fmt]")
{
    DatArchive archive;
    archive.add_file_name("123.txt");
    archive.add_file_name("abc.txt");
    do_test(archive, "tests/fmt/whale/files/dat/encrypted.dat");
}
