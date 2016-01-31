#include "dec/kaguya/wfl_archive_decoder.h"
#include "algo/binary.h"
#include "algo/locale.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::kaguya;

TEST_CASE("Atelier Kaguya WFL archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> &expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
        tests::stub_file("魔法.txt", "expecto patronum"_b),
    };

    io::File input_file;
    input_file.stream.write("WFL1"_b);
    for (const auto &file : expected_files)
    {
        const auto name = algo::unxor(
            algo::utf8_to_sjis(file->path.str()) + "\x00"_b, 0xFF);
        input_file.stream.write_le<u32>(name.size());
        input_file.stream.write(name);
        input_file.stream.write_le<u16>(0);
        input_file.stream.write_le<u32>(file->stream.size());
        input_file.stream.write(file->stream.seek(0).read_to_eof());
    }

    const auto decoder = WflArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}
