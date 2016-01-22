#include "dec/leaf/leafpack_group/leafpack_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::leaf;

static const std::string dir = "tests/dec/leaf/files/leafpack/";

static void do_test(const std::string &input_path, const bstr &key)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("123456789.txt", "1234567890"_b),
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
    };
    LeafpackArchiveDecoder decoder;
    decoder.key = key;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("Leaf LEAFPACK archives", "[dec]")
{
    do_test("test.pak", "\x51\x42\xFE\x77\x2D\x65\x48\x7E\x0A\x8A\xE5"_b);
}
