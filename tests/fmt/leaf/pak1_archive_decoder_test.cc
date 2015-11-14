#include "fmt/leaf/pak1_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::leaf;

TEST_CASE("Leaf PAK archives (version 1)", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::file_from_path("tests/fmt/leaf/files/pak1/leaflogo-out.c16"),
        tests::file_from_path("tests/fmt/leaf/files/pak1/leaflogo-out.grp"),
    };
    expected_files[0]->name = "leaflogo.c16";
    expected_files[1]->name = "leaflogo.grp";

    Pak1ArchiveDecoder decoder;
    decoder.set_version(1);
    auto input_file = tests::file_from_path(
        "tests/fmt/leaf/files/pak1/LEAFLOGO.PAK");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Leaf PAK archives (version 2)", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::file_from_path("tests/fmt/leaf/files/pak1/leaf-out.c16"),
        tests::file_from_path("tests/fmt/leaf/files/pak1/leaf-out.grp"),
    };
    expected_files[0]->name = "leaf.c16";
    expected_files[1]->name = "leaf.grp";

    Pak1ArchiveDecoder decoder;
    decoder.set_version(2);
    auto input_file = tests::file_from_path(
        "tests/fmt/leaf/files/pak1/LEAFLOGO2.PAK");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
