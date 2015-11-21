#include "fmt/team_shanghai_alice/thbgm_audio_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::team_shanghai_alice;

static const std::string dir = "tests/fmt/team_shanghai_alice/files/thbgm/";

static void do_test(
    const ThbgmAudioArchiveDecoder &decoder,
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Team Shanghai Alice THBGM audio", "[fmt]")
{
    SECTION("Manual number of loops")
    {
        ThbgmAudioArchiveDecoder decoder;
        decoder.set_loop_count(0);
        do_test(
            decoder,
            "thbgm-data.dat",
            {
                tests::file_from_path(dir + "0-loops/1-out.wav", "1.wav"),
                tests::file_from_path(dir + "0-loops/2-out.wav", "2.wav"),
            });
    }

    SECTION("Default number of loops")
    {
        const ThbgmAudioArchiveDecoder decoder;
        do_test(
            decoder,
            "thbgm-data.dat",
            {
                tests::file_from_path(dir + "5-loops/1-out.wav", "1.wav"),
                tests::file_from_path(dir + "5-loops/2-out.wav", "2.wav"),
            });
    }
}
