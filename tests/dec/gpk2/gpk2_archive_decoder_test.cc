#include "dec/gpk2/gpk2_archive_decoder.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::gpk2;

static void do_test(
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    io::File input_file;
    input_file.stream.write("GPK2"_b);
    input_file.stream.write_le<u32>(0);
    io::MemoryByteStream table_stream;
    table_stream.write_le<u32>(expected_files.size());
    for (const auto &file : expected_files)
    {
        table_stream.write_le<u32>(input_file.stream.pos());
        table_stream.write_le<u32>(file->stream.size());
        table_stream.write_zero_padded(file->path.str(), 0x80);
        input_file.stream.write(file->stream.seek(0).read_to_eof());
    }
    const auto offset_to_table = input_file.stream.pos();
    input_file.stream.write(table_stream.seek(0).read_to_eof());
    input_file.stream.seek(4).write_le<u32>(offset_to_table);

    const auto decoder = Gpk2ArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("GPK2 GPK2 archives", "[dec]")
{
    do_test(
        {
            tests::stub_file("00000.dat", "1234567890"_b),
            tests::stub_file("00001.dat", "abcdefghijklmnopqrstuvwxyz"_b),
        });
}
