#include "test_support/audio_support.h"
#include <algorithm>
#include "fmt/microsoft/wav_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;

static res::Audio audio_from_file(io::File &file)
{
    const fmt::microsoft::WavAudioDecoder wav_audio_decoder;
    return wav_audio_decoder.decode(file);
}

void tests::compare_audio(
    const res::Audio &expected, const res::Audio &actual)
{
    REQUIRE(expected.codec == actual.codec);
    REQUIRE(expected.channel_count == actual.channel_count);
    REQUIRE(expected.sample_rate == actual.sample_rate);
    REQUIRE(expected.bits_per_sample == actual.bits_per_sample);
    REQUIRE(expected.extra_codec_headers == actual.extra_codec_headers);
    REQUIRE(expected.samples.size() == actual.samples.size());
    REQUIRE(expected.samples == actual.samples);

    REQUIRE(expected.loops.size() == actual.loops.size());
    for (const auto i : util::range(expected.loops.size()))
    {
        const auto &expected_loop = expected.loops[i];
        const auto &actual_loop = actual.loops[i];
        REQUIRE(expected_loop.start == actual_loop.start);
        REQUIRE(expected_loop.end == actual_loop.end);
        REQUIRE(expected_loop.play_count == actual_loop.play_count);
    }
}

void tests::compare_audio(
    io::File &expected_file, const res::Audio &actual_audio)
{
    tests::compare_audio(audio_from_file(expected_file), actual_audio);
}

void tests::compare_audio(
    io::File &expected_file,
    io::File &actual_file,
    const bool compare_file_names)
{
    auto expected_audio = audio_from_file(expected_file);
    auto actual_audio = audio_from_file(actual_file);
    if (compare_file_names)
        tests::compare_file_names(expected_file.name, actual_file.name);
    tests::compare_audio(expected_audio, actual_audio);
}

void tests::compare_audio(
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const std::vector<std::shared_ptr<io::File>> &actual_files,
    const bool compare_file_names)
{
    REQUIRE(expected_files.size() == actual_files.size());
    for (const auto i : util::range(actual_files.size()))
    {
        INFO(util::format("Audio at index %d differs", i));
        tests::compare_audio(
            *expected_files[i], *actual_files[i], compare_file_names);
    }
}
