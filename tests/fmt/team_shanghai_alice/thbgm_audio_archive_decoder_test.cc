#include "fmt/team_shanghai_alice/thbgm_audio_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::team_shanghai_alice;

TEST_CASE("Team Shanghai Alice THBGM audio (manual number of loops)", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::file_from_path(
            "tests/fmt/team_shanghai_alice/files/thbgm/0-loops/1-out.wav"),
        tests::file_from_path(
            "tests/fmt/team_shanghai_alice/files/thbgm/0-loops/2-out.wav"),
    };
    expected_files[0]->name = "1.wav";
    expected_files[1]->name = "2.wav";

    ThbgmAudioArchiveDecoder decoder;
    decoder.set_loop_count(0);
    const auto input_file = tests::file_from_path(
        "tests/fmt/team_shanghai_alice/files/thbgm/thbgm-data.dat");
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Team Shanghai Alice THBGM audio (default number of loops)", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::file_from_path(
            "tests/fmt/team_shanghai_alice/files/thbgm/5-loops/1-out.wav"),
        tests::file_from_path(
            "tests/fmt/team_shanghai_alice/files/thbgm/5-loops/2-out.wav"),
    };
    expected_files[0]->name = "1.wav";
    expected_files[1]->name = "2.wav";

    const ThbgmAudioArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/team_shanghai_alice/files/thbgm/thbgm-data.dat");
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
