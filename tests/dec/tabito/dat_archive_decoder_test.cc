#include "dec/tabito/dat_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::tabito;

static std::unique_ptr<io::File> get_dat_file(
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    auto output_file = std::make_unique<io::File>("test.dat", ""_b);
    for (const auto &file : expected_files)
    {
        output_file->stream.write_le<u32>(file->stream.size());
        output_file->stream.write("JUNK"_b);
        output_file->stream.write_le<u16>(file->path.str().size());
        output_file->stream.write(file->path.str());
        output_file->stream.write(file->stream.seek(0));
    }
    output_file->stream.write_le<u32>(0);
    return output_file;
}

TEST_CASE("Tabito DAT archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    auto input_file = get_dat_file(expected_files);
    const auto decoder = DatArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}
