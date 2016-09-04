#include "dec/dogenzaka/a_file_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::dogenzaka;

static const std::string dir = "tests/dec/dogenzaka/files/a/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = AFileDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*actual_file, *expected_file, false);
}

TEST_CASE("Dogenzaka A files", "[dec]")
{
    do_test("BG_005_000.a", "BG_005_000-out.png");
}
