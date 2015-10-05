#include "fmt/leaf/pak_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::leaf;

TEST_CASE("Leaf PAK archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::file_from_path("tests/fmt/leaf/files/pak/leaflogo-out.c16"),
        tests::file_from_path("tests/fmt/leaf/files/pak/leaflogo-out.grp"),
    };
    expected_files[0]->name = "leaflogo.c16";
    expected_files[1]->name = "leaflogo.grp";

    PakArchiveDecoder decoder;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/leaf/files/pak/LEAFLOGO.PAK", decoder);

    tests::compare_files(expected_files, actual_files, true);
}
