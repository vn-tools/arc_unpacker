#include "fmt/touhou/thbgm_audio_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::touhou;

TEST_CASE("Touhou THBGM audio (manual number of loops)", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::file_from_path("tests/fmt/touhou/files/thbgm/0-loops/1-out.wav"),
        tests::file_from_path("tests/fmt/touhou/files/thbgm/0-loops/2-out.wav"),
    };
    expected_files[0]->name = "1.wav";
    expected_files[1]->name = "2.wav";

    ThbgmAudioArchiveDecoder decoder;
    decoder.set_loop_count(0);
    auto input_file = tests::file_from_path(
        "tests/fmt/touhou/files/thbgm/thbgm-data.dat");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Touhou THBGM audio (default number of loops)", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::file_from_path("tests/fmt/touhou/files/thbgm/5-loops/1-out.wav"),
        tests::file_from_path("tests/fmt/touhou/files/thbgm/5-loops/2-out.wav"),
    };
    expected_files[0]->name = "1.wav";
    expected_files[1]->name = "2.wav";

    ThbgmAudioArchiveDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/touhou/files/thbgm/thbgm-data.dat");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
