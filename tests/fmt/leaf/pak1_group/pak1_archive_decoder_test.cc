#include "fmt/leaf/pak1_group/pak1_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::leaf;

static const std::string dir = "tests/fmt/leaf/files/pak1/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    int version)
{
    Pak1ArchiveDecoder decoder;
    decoder.set_version(version);
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Leaf PAK1 archives", "[fmt]")
{
    SECTION("Version 1")
    {
        do_test(
            "LEAFLOGO.PAK",
            {
                tests::file_from_path(dir + "leaflogo-out.c16", "leaflogo.c16"),
                tests::file_from_path(dir + "leaflogo-out.grp", "leaflogo.grp"),
            },
            1);
    }

    SECTION("Version 2")
    {
        do_test(
            "LEAFLOGO2.PAK",
            {
                tests::file_from_path(dir + "leaf-out.c16", "leaf.c16"),
                tests::file_from_path(dir + "leaf-out.grp", "leaf.grp"),
            },
            2);
    }
}
