#include "fmt/twilight_frontier/pak1_audio_archive_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

static const std::string dir = "tests/fmt/twilight_frontier/files/pak1/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const Pak1AudioArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_audio(expected_files, actual_files, true);
}

TEST_CASE("Twilight Frontier PAK1 audio", "[fmt]")
{
    do_test(
        "se.dat",
        {
            tests::file_from_path(dir + "se-0001-out.wav", "0001.wav"),
            tests::file_from_path(dir + "se-0002-out.wav", "0002.wav"),
        });
}
