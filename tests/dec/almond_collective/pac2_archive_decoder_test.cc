#include "dec/almond_collective/pac2_archive_decoder.h"
#include "algo/pack/zlib.h"
#include "io/memory_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::almond_collective;

static std::unique_ptr<io::File> create_file(
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const bstr &key)
{
    auto output_file = std::make_unique<io::File>();
    output_file->stream.write("PAC2"_b);
    output_file->stream.write("JUNK"_b);
    output_file->stream.write("JUNK"_b);
    output_file->stream.write("CCOD"_b);
    output_file->stream.write_le<u32>(key.size());
    output_file->stream.write(key);
    output_file->stream.write("FNUM"_b);
    output_file->stream.write_le<u32>(4);
    output_file->stream.write_le<u32>(expected_files.size());

    for (const auto &file : expected_files)
    {
        auto data = file->stream.seek(0).read_to_eof();
        const auto name = file->path.str();
        const auto entry_size = 16 + name.size() + data.size();
        output_file->stream.write("FILE"_b);
        output_file->stream.write_le<u32>(entry_size);
        output_file->stream.write("NAME"_b);
        output_file->stream.write_le<u32>(name.size());
        output_file->stream.write(name);
        output_file->stream.write("DATA"_b);
        output_file->stream.write_le<u32>(data.size());
        auto key_pos = output_file->stream.pos();
        for (const auto i : algo::range(data.size()))
            data[i] ^= key[(key_pos++) % key.size()];
        output_file->stream.write(data);
    }
    return output_file;
}

TEST_CASE("Almond Collective PAC2 archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
        {
            tests::stub_file("123.txt", "1234567890"_b),
            tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
        };

    std::unique_ptr<io::File> input_file;

    input_file = create_file(expected_files, "herpderp"_b);

    const auto decoder = Pac2ArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}
