#include "fmt/lilim/aos1_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::lilim;

static void do_test(const std::string &input_path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
    };
    for (const auto i : util::range(50))
        expected_files.push_back(tests::stub_file(
            util::format("extra%d.txt", i),
            bstr(util::format("content%d", i))));

    const Aos1ArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Lilim AOS1 archives", "[fmt]")
{
    do_test("tests/fmt/lilim/files/aos1/test.aos");
}
