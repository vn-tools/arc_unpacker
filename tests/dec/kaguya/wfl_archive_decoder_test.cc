#include "dec/kaguya/wfl_archive_decoder.h"
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/pack/lzss.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::kaguya;

static bstr compress(const bstr &input)
{
    algo::pack::BitwiseLzssSettings settings;
    settings.min_match_size = 2;
    settings.position_bits = 12;
    settings.size_bits = 4;
    settings.initial_dictionary_pos = 1;
    return algo::pack::lzss_compress(input, settings);
}

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

    SECTION("Uncompressed")
    {
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
    }

    SECTION("Compressed")
    {
        for (const auto &file : expected_files)
        {
            const auto name = algo::unxor(
                algo::utf8_to_sjis(file->path.str()) + "\x00"_b, 0xFF);
            input_file.stream.write_le<u32>(name.size());
            input_file.stream.write(name);
            const auto data_orig = file->stream.seek(0).read_to_eof();
            const auto data_comp = compress(data_orig);
            input_file.stream.write_le<u16>(1);
            input_file.stream.write_le<u32>(data_comp.size());
            input_file.stream.write_le<u32>(data_orig.size());
            input_file.stream.write(data_comp);
        }
    }

    const auto decoder = WflArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}
