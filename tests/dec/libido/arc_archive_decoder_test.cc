#include "dec/libido/arc_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::libido;

static const std::string dir = "tests/dec/libido/files/arc/";

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890 123 456789 0"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };
    const auto decoder = ArcArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("Libido ARC archives", "[dec]")
{
    SECTION("Plain")
    {
        do_test("unencrypted.arc");
    }

    SECTION("Encrypted")
    {
        do_test("encrypted.arc");
    }
}
