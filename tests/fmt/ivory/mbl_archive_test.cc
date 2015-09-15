#include "fmt/ivory/mbl_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::ivory;

static void do_test(const std::string &path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "abc"_b),
        tests::stub_file("テスト", "AAAAAAAAAAAAAAAA"_b),
    };

    MblArchive archive;
    auto actual_files = au::tests::unpack_to_memory(path, archive);

    tests::compare_files(expected_files, actual_files, true);
}

static void do_test_encrypted(
    const std::string &input_arc_path,
    const std::string &expected_file_path,
    const std::string &plugin_name)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::file_from_path(expected_file_path),
    };

    MblArchive archive;
    archive.set_plugin(plugin_name);
    auto actual_files = au::tests::unpack_to_memory(input_arc_path, archive);

    tests::compare_files(expected_files, actual_files, false);
}

TEST_CASE("Unpacking version 1 MBL archives works", "[fmt]")
{
    do_test("tests/fmt/ivory/files/mbl/mbl-v1.mbl");
}

TEST_CASE("Unpacking version 2 MBL archives works", "[fmt]")
{
    do_test("tests/fmt/ivory/files/mbl/mbl-v2.mbl");
}

TEST_CASE("Unpacking encrypted Candy Toys dialog MBL archives works", "[fmt]")
{
    do_test_encrypted(
        "tests/fmt/ivory/files/mbl/mg_data-candy.mbl",
        "tests/fmt/ivory/files/mbl/MAIN-candy",
        "candy");
}

TEST_CASE(
    "Unpacking encrypted Wanko to Kurasou dialog MBL archives works", "[fmt]")
{
    do_test_encrypted(
        "tests/fmt/ivory/files/mbl/mg_data-wanko.mbl",
        "tests/fmt/ivory/files/mbl/TEST-wanko",
        "wanko");
}
