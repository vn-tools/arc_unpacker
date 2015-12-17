#include "fmt/ivory/mbl_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::ivory;

static const std::string dir = "tests/fmt/ivory/files/mbl/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const std::string &plugin_name = "")
{
    MblArchiveDecoder decoder;
    if (!plugin_name.empty())
        decoder.set_plugin(plugin_name);
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Ivory MBL archives", "[fmt]")
{
    SECTION("Version 1")
    {
        do_test(
            "mbl-v1.mbl",
            {
                tests::stub_file("abc.txt", "abc"_b),
                tests::stub_file("テスト", "AAAAAAAAAAAAAAAA"_b),
            });
    }

    SECTION("Version 2")
    {
        do_test(
            "mbl-v2.mbl",
            {
                tests::stub_file("abc.txt", "abc"_b),
                tests::stub_file("テスト", "AAAAAAAAAAAAAAAA"_b),
            });
    }

    SECTION("Dialogs, Candy Toys encryption")
    {
        do_test(
            "mg_data-candy.mbl",
            {tests::file_from_path(dir + "MAIN-candy", "MAIN")},
            "candy");
    }

    SECTION("Dialogs, Wanko to Kurasou encryption")
    {
        do_test(
            "mg_data-wanko.mbl",
            {tests::file_from_path(dir + "TEST-wanko", "TEST")},
            "wanko");
    }
}
