#include "dec/qlie/abmp7_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::qlie;

static const std::string dir = "tests/dec/qlie/files/abmp7/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const auto decoder = Abmp7ArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("QLiE ABMP7 archives", "[dec]")
{
    do_test(
        "button.b",
        {
            tests::file_from_path(dir + "out/base.png", "base.png"),

            tests::file_from_path(
                dir + "out/button-sound-effect-1.ogg", u8"ボタン効果音1.ogg"),

            tests::file_from_path(
                dir + "out/button-sound-effect-2.ogg", u8"ボタン効果音2.ogg"),
        });
}
