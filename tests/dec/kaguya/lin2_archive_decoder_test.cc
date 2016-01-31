#include "dec/kaguya/lin2_archive_decoder.h"
#include "algo/binary.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::kaguya;

TEST_CASE("Atelier Kaguya LIN2 archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> &expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    const auto header_size = 8;
    auto table_size = 0u;
    for (const auto &file : expected_files)
        table_size += 2 + file->path.str().size() + 1 + 10;
    const auto data_offset = header_size + table_size;

    io::File input_file;
    input_file.stream.write("LIN2"_b);
    input_file.stream.write_le<u32>(expected_files.size());
    auto offset = data_offset;
    for (const auto &file : expected_files)
    {
        input_file.stream.write_le<u16>(file->path.str().size() + 1);
        input_file.stream.write(algo::unxor(file->path.str(), 0xFF));
        input_file.stream.write<u8>(0xFF);
        input_file.stream.write_le<u32>(offset);
        input_file.stream.write_le<u32>(file->stream.size());
        input_file.stream.write("??"_b);
        offset += file->stream.size();
    }
    REQUIRE(input_file.stream.tell() == data_offset);
    for (const auto &file : expected_files)
        input_file.stream.write(file->stream.seek(0).read_to_eof());

    const auto decoder = Lin2ArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}
