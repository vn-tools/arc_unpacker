#include "fmt/libido/arc_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::libido;

static void do_test(const std::string &input_path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890 123 456789 0"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    ArcArchiveDecoder decoder;
    auto actual_files = tests::unpack_to_memory(input_path, decoder);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Libido ARC unencrypted archives", "[fmt]")
{
    do_test("tests/fmt/libido/files/arc/unencrypted.arc");
}

TEST_CASE("Libido ARC encrypted archives", "[fmt]")
{
    do_test("tests/fmt/libido/files/arc/encrypted.arc");
}
