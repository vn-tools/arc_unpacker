#include "dec/abogado/dsk_archive_decoder.h"
#include "algo/locale.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::abogado;

static constexpr u32 mask = ((1 << 11) - 1);

static std::unique_ptr<io::File> get_dsk_file(
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    auto output_file = std::make_unique<io::File>("test.DSK", ""_b);
    for (const auto &file : expected_files)
    {
        output_file->stream.write(file->stream.seek(0));
        while (output_file->stream.pos() & mask)
            output_file->stream.write<u8>('-');
    }
    return output_file;
}

static std::unique_ptr<io::File> get_pft_file(
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    auto output_file = std::make_unique<io::File>("test.PFT", ""_b);
    output_file->stream.write("\x10\x00\x00\x08"_b);
    output_file->stream.write_le<u32>(expected_files.size());
    output_file->stream.write("JUNK"_b);
    output_file->stream.write("JUNK"_b);

    auto current_offset = 0_z;
    for (const auto &file : expected_files)
    {
        auto size_padded = file->stream.size();
        while (size_padded & mask)
            size_padded++;

        output_file->stream.write_zero_padded(file->path.str(), 8);
        output_file->stream.write_le<u32>(current_offset >> 11);
        output_file->stream.write_le<u32>(file->stream.size());

        current_offset += size_padded;
    }

    return output_file;
}

TEST_CASE("Abogado DSK+PFT archives", "[dec]")
{
    const std::vector<std::shared_ptr<io::File>> expected_files =
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    auto input_file = get_dsk_file(expected_files);
    VirtualFileSystem::register_file(
        "test.PFT",
        [&]() { return get_pft_file(expected_files); });

    const auto decoder = DskArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}
