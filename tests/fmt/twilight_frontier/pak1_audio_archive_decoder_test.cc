#include "fmt/twilight_frontier/pak1_audio_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

TEST_CASE("Twilight Frontier PAK1 audio", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::file_from_path(
            "tests/fmt/twilight_frontier/files/pak1/se-0001-out.wav"),
        tests::file_from_path(
            "tests/fmt/twilight_frontier/files/pak1/se-0002-out.wav"),
    };
    expected_files[0]->name = "0001.wav";
    expected_files[1]->name = "0002.wav";

    Pak1AudioArchiveDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/twilight_frontier/files/pak1/se.dat");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
