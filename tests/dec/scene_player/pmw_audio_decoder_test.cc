#include "dec/scene_player/pmw_audio_decoder.h"
#include "algo/binary.h"
#include "algo/pack/zlib.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::scene_player;

TEST_CASE("ScenePlayer PMW audio", "[dec]")
{
    Logger dummy_logger;
    dummy_logger.mute();
    const auto input_text = "some .wav content"_b;
    io::File expected_file("test.wav", input_text);
    io::File input_file(
        "test.pmw",
        algo::unxor(
            algo::pack::zlib_deflate(
                input_text,
                algo::pack::ZlibKind::PlainZlib,
                algo::pack::CompressionLevel::Store),
            0x21));

    const auto decoder = PmwAudioDecoder();
    const auto actual_file = tests::decode(decoder, input_file);
    tests::compare_files(expected_file, *actual_file, true);
}
