#include "dec/real_live/koepac_audio_archive_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::real_live;

static const io::path dir = "tests/dec/real_live/files/koepac/";

static void do_test(
    const io::path &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const auto decoder = KoepacAudioArchiveDecoder();
    const auto input_file = tests::file_from_path(input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_audio(actual_files, expected_files, true);
}

TEST_CASE("RealLive KOEPAC archives", "[dec]")
{
    do_test(
        dir / "Z5000.KOE",
        {
            tests::zlib_file_from_path(
                dir / "sample00016-out-zlib.wav",
                "sample00016.wav"),

            tests::zlib_file_from_path(
                dir / "sample00034-out-zlib.wav",
                "sample00034.wav"),
        });
}
