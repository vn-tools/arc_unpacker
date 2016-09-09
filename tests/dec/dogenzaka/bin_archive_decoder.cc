#include "dec/dogenzaka/bin_archive_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::dogenzaka;

TEST_CASE("Dogenzaka BIN archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    auto input_file = std::make_unique<io::File>("test.bin", ""_b);
    input_file->stream.write_le<u32>(expected_files.size());

    SECTION("Offset only")
    {
        auto offset = 0;
        for (const auto &file : expected_files)
        {
            input_file->stream.write_le<u32>(offset);
            offset += file->stream.size();
        }
        for (const auto &file : expected_files)
            input_file->stream.write(file->stream.seek(0));
    }

    SECTION("Offset and size pairs")
    {
        const auto header_size = 12;
        auto offset = 8 * expected_files.size() + 4;
        for (const auto &file : expected_files)
        {
            input_file->stream.write_le<u32>(offset);
            input_file->stream.write_le<u32>(file->stream.size() + header_size);
            offset += file->stream.size() + header_size;
        }
        for (const auto &file : expected_files)
        {
            // This kind of archive also uses headers of variable size,
            // probably identified by file type types. For files of any
            // significance, these headers are always 12 byte long.
            for (const auto i : algo::range(header_size))
                input_file->stream.write("JUNK"_b[i % 4]);
            input_file->stream.write(file->stream.seek(0));
        }
    }

    const auto decoder = BinArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, false);
}
