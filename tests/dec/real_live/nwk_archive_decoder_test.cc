#include "dec/real_live/nwk_archive_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::real_live;

TEST_CASE("RealLive NWK archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> &expected_files =
    {
        tests::stub_file("sample00000.nwa", "1234567890"_b),
        tests::stub_file("sample00001.nwa", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    io::File input_file("test.nwk", ""_b);
    input_file.stream.write_le<u32>(expected_files.size());
    auto offset = 4 + 12 * expected_files.size();
    for (const auto i : algo::range(expected_files.size()))
    {
        const auto &expected_file = expected_files[i];
        input_file.stream.write_le<u32>(expected_file->stream.size());
        input_file.stream.write_le<u32>(offset);
        input_file.stream.write_le<u32>(i);
        offset += expected_file->stream.size();
    }
    for (auto &expected_file : expected_files)
        input_file.stream.write(expected_file->stream.seek(0).read_to_eof());

    const auto decoder = NwkArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_files(actual_files, expected_files, true);
}
