#include "dec/ast/arc_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::ast;

static const std::string dir = "tests/dec/ast/files/arc/";

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("123.txt", "RIFF123123123 123123123 321"_b),
        tests::stub_file("abc.xyz", "RIFFabcdefghijklmnopqrstuvwxyz"_b),
    };
    const auto decoder = ArcArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("AST ARC archives", "[dec]")
{
    SECTION("Version 1")
    {
        do_test("test-v1");
    }

    SECTION("Version 2, plain")
    {
        do_test("test-v2-plain");
    }

    SECTION("Version 2, compressed")
    {
        do_test("test-v2-compressed");
    }

    SECTION("Version 2, encrypted")
    {
        do_test("test-v2-encrypted");
    }
}
