#include "fmt/libido/arc_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::libido;

TEST_CASE("Unpacking Libido's ARC archives works")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::zlib_file_from_path(
            "tests/fmt/libido/files/arc/Game9999-zlib.bmp"),
    };
    expected_files[0]->name = "Game9999.bmp";

    ArcArchive archive;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/libido/files/arc/test.arc", archive);

    tests::compare_files(expected_files, actual_files, true);
}
