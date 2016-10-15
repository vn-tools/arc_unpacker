#include "dec/pajamas/gamedat_archive_decoder.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::pajamas;

static std::unique_ptr<io::File> get_gamedat_file(
    const int version,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    auto output_file = std::make_unique<io::File>("test.dat", ""_b);
    output_file->stream.write("GAMEDAT PAC"_b);
    output_file->stream.write<u8>(version == 1 ? 'K' : '2');
    output_file->stream.write_le<u32>(expected_files.size());
    const auto name_size = version == 1 ? 16 : 32;
    for (const auto &file : expected_files)
        output_file->stream.write_zero_padded(file->path.str(), name_size);
    auto data_offset = 0_z;
    for (const auto &file : expected_files)
    {
        output_file->stream.write_le<u32>(data_offset);
        output_file->stream.write_le<u32>(file->stream.size());
        data_offset += file->stream.size();
    }
    for (const auto &file : expected_files)
        output_file->stream.write(file->stream.seek(0).read_to_eof());
    return output_file;
}

TEST_CASE("Pajamas GAMEDAT archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    std::unique_ptr<io::File> input_file;
    SECTION("Version 1")
    {
        input_file = get_gamedat_file(1, expected_files);
    }
    SECTION("Version 2")
    {
        input_file = get_gamedat_file(2, expected_files);
    }

    const auto decoder = GamedatArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}
