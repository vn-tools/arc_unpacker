#include "fmt/yuka_script/ykc_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::yuka_script;

TEST_CASE("Unpacking YKC archives works")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::create_file("abc.txt", "123"_b),
        tests::create_file("another.txt", "abcdefghij"_b),
    };

    YkcArchive archive;
    tests::compare_files(
        expected_files,
        tests::unpack_to_memory(
            "tests/fmt/yuka_script/files/test.ykc", archive),
        true);
}
