#include "dec/gsd/gsp_archive_decoder.h"
#include "algo/binary.h"
#include "algo/ptr.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::gsd;

TEST_CASE("GSD GSP archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> &expected_files =
    {
        tests::stub_file("00000.dat", "1234567890"_b),
        tests::stub_file("00001.dat", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    io::File input_file;
    input_file.stream.write_le<u32>(expected_files.size());
    auto offset = 4 + expected_files.size() * 0x40;
    for (const auto &file : expected_files)
    {
        input_file.stream.write_le<u32>(offset);
        input_file.stream.write_le<u32>(file->stream.size());
        input_file.stream.write_zero_padded(file->path.str(), 0x38);
        offset += file->stream.size();
    }
    for (const auto &file : expected_files)
        input_file.stream.write(file->stream.seek(0).read_to_eof());

    const auto decoder = GspArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}
