#include "test_support/audio_support.h"
#include "algo/format.h"
#include "algo/range.h"
#include "dec/microsoft/wav_audio_decoder.h"
#include "test_support/catch.h"
#include "test_support/file_support.h"

using namespace au;

static res::Audio audio_from_file(io::File &file)
{
    Logger dummy_logger;
    dummy_logger.mute();
    const auto wav_audio_decoder = dec::microsoft::WavAudioDecoder();
    return wav_audio_decoder.decode(dummy_logger, file);
}

void tests::compare_audio(
    const res::Audio &actual, const res::Audio &expected)
{
    REQUIRE(expected.codec == actual.codec);
    REQUIRE(expected.channel_count == actual.channel_count);
    REQUIRE(expected.sample_rate == actual.sample_rate);
    REQUIRE(expected.bits_per_sample == actual.bits_per_sample);
    REQUIRE(expected.extra_codec_headers == actual.extra_codec_headers);
    REQUIRE(expected.samples.size() == actual.samples.size());
    REQUIRE(expected.samples == actual.samples);

    REQUIRE(expected.loops.size() == actual.loops.size());
    for (const auto i : algo::range(expected.loops.size()))
    {
        const auto &expected_loop = expected.loops[i];
        const auto &actual_loop = actual.loops[i];
        REQUIRE(expected_loop.start == actual_loop.start);
        REQUIRE(expected_loop.end == actual_loop.end);
        REQUIRE(expected_loop.play_count == actual_loop.play_count);
    }
}

void tests::compare_audio(
    const res::Audio &actual_audio, io::File &expected_file)
{
    tests::compare_audio(actual_audio, audio_from_file(expected_file));
}

void tests::compare_audio(
    io::File &actual_file,
    io::File &expected_file,
    const bool compare_file_paths)
{
    auto expected_audio = audio_from_file(expected_file);
    auto actual_audio = audio_from_file(actual_file);
    if (compare_file_paths)
        tests::compare_paths(actual_file.path, expected_file.path);
    tests::compare_audio(actual_audio, expected_audio);
}

void tests::compare_audio(
    const std::vector<std::shared_ptr<io::File>> &actual_files,
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const bool compare_file_paths)
{
    REQUIRE(expected_files.size() == actual_files.size());
    for (const auto i : algo::range(actual_files.size()))
    {
        INFO(algo::format("Audio at index %d differs", i));
        tests::compare_audio(
            *actual_files[i], *expected_files[i], compare_file_paths);
    }
}
