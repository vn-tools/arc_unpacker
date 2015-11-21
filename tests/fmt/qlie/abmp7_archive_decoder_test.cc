#include "fmt/qlie/abmp7_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::qlie;

static const std::string dir = "tests/fmt/qlie/files/abmp7/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const Abmp7ArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("QLiE ABMP7 archives", "[fmt]")
{
    do_test(
        "ボタン.b",
        {
            tests::file_from_path(dir + "out/base.png", "base.png"),
            tests::file_from_path(
                dir + "out/ボタン効果音1.ogg", "ボタン効果音1.ogg"),
            tests::file_from_path(
                dir + "out/ボタン効果音2.ogg", "ボタン効果音2.ogg"),
        });
}
