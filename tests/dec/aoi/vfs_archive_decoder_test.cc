#include "dec/aoi/vfs_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::aoi;

TEST_CASE("Aoi VFS archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> &expected_files =
        {
            tests::stub_file("123.txt", "1234567890"_b),
            tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
        };

    io::File input_file;
    const auto entry_size = 0x13 + 8;
    const auto table_size = entry_size * expected_files.size();
    input_file.stream.write("VF\x00\x01"_b);
    input_file.stream.write_le<u16>(expected_files.size());
    input_file.stream.write_le<u16>(entry_size);
    input_file.stream.write_le<u32>(table_size);
    input_file.stream.write("JUNK"_b);
    size_t offset = input_file.stream.tell() + table_size;
    for (const auto &file : expected_files)
    {
        input_file.stream.write_zero_padded(file->path.str(), 0x13);
        input_file.stream.write_le<u32>(offset);
        input_file.stream.write_le<u32>(file->stream.size());
        offset += file->stream.size();
    }
    for (const auto &file : expected_files)
        input_file.stream.write(file->stream.seek(0).read_to_eof());

    const auto decoder = VfsArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}
