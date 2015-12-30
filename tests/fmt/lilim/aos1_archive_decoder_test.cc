#include "fmt/lilim/aos1_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::lilim;

static const std::string dir = "tests/fmt/lilim/files/aos1/";

static void do_test(const std::string &input_path)
{
    std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
    };
    for (const auto i : algo::range(50))
        expected_files.push_back(tests::stub_file(
            algo::format("extra%d.txt", i),
            bstr(algo::format("content%d", i))));

    const auto decoder = Aos1ArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Lilim AOS1 archives", "[fmt]")
{
    do_test("test.aos");
}
