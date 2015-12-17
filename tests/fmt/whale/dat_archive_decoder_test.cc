#include "fmt/whale/dat_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::whale;

static const std::string dir = "tests/fmt/whale/files/dat/";

static void do_test(
    DatArchiveDecoder &decoder,
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Whale DAT archives", "[fmt]")
{
    SECTION("Plain")
    {
        DatArchiveDecoder decoder;
        decoder.add_file_name("123.txt");
        decoder.add_file_name("abc.txt");
        do_test(
            decoder,
            "plain.dat",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }

    SECTION("Compressed")
    {
        DatArchiveDecoder decoder;
        decoder.add_file_name("123.txt");
        decoder.add_file_name("abc.txt");
        decoder.set_game_title("A Dog Story");
        do_test(
            decoder,
            "compressed.dat",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }

    SECTION("Compressed, unknown names")
    {
        DatArchiveDecoder decoder;
        decoder.set_game_title("A Dog Story");
        do_test(
            decoder,
            "compressed.dat",
            {
                tests::stub_file("0000.txt", "1234567890"_b),
                tests::stub_file("0001.txt", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }

    SECTION("Fully encrypted")
    {
        DatArchiveDecoder decoder;
        decoder.add_file_name("123.txt");
        decoder.add_file_name("abc.txt");
        do_test(
            decoder,
            "encrypted.dat",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }
}
