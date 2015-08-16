#include "fmt/renpy/rpa_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::renpy;

// import cPickle
// import zlib
//
// key = 0
// def filter(x):
//   return x ^ key
//
// print zlib.compress(cPickle.dumps({
//   'abc.txt': [(filter(0x19), filter(2), '1')],
//   'another.txt': [(filter(0x1B), filter(7), 'abc')]
// }, cPickle.HIGHEST_PROTOCOL))

static void test(const std::string &path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::create_file("another.txt", "abcdefghij"_b),
        tests::create_file("abc.txt", "123"_b),
    };

    RpaArchive archive;
    tests::compare_files(
        expected_files, tests::unpack_to_memory(path, archive), true);
}

TEST_CASE("Unpacking version 3 RPA archives works")
{
    test("tests/fmt/renpy/files/v3.rpa");
}

TEST_CASE("Unpacking version 2 RPA archives works")
{
    test("tests/fmt/renpy/files/v2.rpa");
}

TEST_CASE("Unpacking RPA archives using data prefixes works")
{
    test("tests/fmt/renpy/files/prefixes.rpa");
}
