#include "fmt/yuka_script/ykc_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::yuka_script;

TEST_CASE("YukaScript YKC archives", "[fmt]")
{
    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("another.txt", "abcdefghij"_b),
    };

    const YkcArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/yuka_script/files/ykc/test.ykc");
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
