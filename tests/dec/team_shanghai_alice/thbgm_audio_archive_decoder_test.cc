#include "dec/team_shanghai_alice/thbgm_audio_archive_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::team_shanghai_alice;

static const std::string dir = "tests/dec/team_shanghai_alice/files/thbgm/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const auto decoder = ThbgmAudioArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_audio(expected_files, actual_files, true);
}

TEST_CASE("Team Shanghai Alice THBGM audio", "[dec]")
{
    do_test(
        "thbgm-data.dat",
        {
            tests::file_from_path(dir + "/1-out.wav", "1.wavloop"),
            tests::file_from_path(dir + "/2-out.wav", "2.wavloop"),
        });
}
