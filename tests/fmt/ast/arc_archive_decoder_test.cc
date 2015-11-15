#include "fmt/ast/arc_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::ast;

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "RIFF123123123 123123123 321"_b),
        tests::stub_file("abc.xyz", "RIFFabcdefghijklmnopqrstuvwxyz"_b),
    };

    const ArcArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("AST ARC1 archives", "[fmt]")
{
    do_test("tests/fmt/ast/files/arc/test-v1");
}

TEST_CASE("AST ARC2 plain archives", "[fmt]")
{
    do_test("tests/fmt/ast/files/arc/test-v2-plain");
}

TEST_CASE("AST ARC2 compressed archives", "[fmt]")
{
    do_test("tests/fmt/ast/files/arc/test-v2-compressed");
}

TEST_CASE("AST ARC2 encrypted archives", "[fmt]")
{
    do_test("tests/fmt/ast/files/arc/test-v2-encrypted");
}
