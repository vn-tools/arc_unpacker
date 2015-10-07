#include "fmt/qlie/abmp7_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::qlie;

TEST_CASE("QLiE ABMP7 archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::file_from_path("tests/fmt/qlie/files/abmp7/out/base.png"),
        tests::file_from_path(
            "tests/fmt/qlie/files/abmp7/out/ボタン効果音1.ogg"),
        tests::file_from_path(
            "tests/fmt/qlie/files/abmp7/out/ボタン効果音2.ogg"),
    };

    expected_files[0]->name = "base.png";
    expected_files[1]->name = "ボタン効果音1.ogg";
    expected_files[2]->name = "ボタン効果音2.ogg";

    Abmp7ArchiveDecoder decoder;
    auto actual_files = au::tests::unpack_to_memory(
        "tests/fmt/qlie/files/abmp7/ボタン.b", decoder);

    tests::compare_files(expected_files, actual_files, true);
}
