#include "dec/crowd/pkwv_audio_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::crowd;

static const std::string dir = "tests/dec/crowd/files/pkwv/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const auto decoder = PkwvAudioArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Crowd PKWV audio archives", "[dec]")
{
    do_test("test.pck",
        {
            tests::file_from_path(dir + "test~.pck/apper0.wav", "apper0.wav"),
            tests::file_from_path(dir + "test~.pck/attack0.wav", "attack0.wav"),
        });
}
