#include "fmt/renpy/rpa_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
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
        tests::stub_file("another.txt", "abcdefghij"_b),
        tests::stub_file("abc.txt", "123"_b),
    };

    RpaArchiveDecoder decoder;
    auto input_file = tests::file_from_path(path);
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Ren'py RPA v3 archives", "[fmt]")
{
    test("tests/fmt/renpy/files/rpa/v3.rpa");
}

TEST_CASE("Ren'py RPA v2 archives", "[fmt]")
{
    test("tests/fmt/renpy/files/rpa/v2.rpa");
}

TEST_CASE("Ren'py RPA archives using data prefixes", "[fmt]")
{
    test("tests/fmt/renpy/files/rpa/prefixes.rpa");
}
