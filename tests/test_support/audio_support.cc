#include "test_support/audio_support.h"
#include <algorithm>
#include "fmt/microsoft/wav_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;

static sfx::Wave audio_from_file(io::File &file)
{
    const fmt::microsoft::WavAudioDecoder wav_audio_decoder;
    return wav_audio_decoder.decode(file);
}

void tests::compare_audio(
    const sfx::Wave &expected, const sfx::Wave &actual)
{
    REQUIRE(expected.fmt.pcm_type == actual.fmt.pcm_type);
    REQUIRE(expected.fmt.channel_count == actual.fmt.channel_count);
    REQUIRE(expected.fmt.sample_rate == actual.fmt.sample_rate);
    REQUIRE(expected.fmt.bits_per_sample == actual.fmt.bits_per_sample);
    REQUIRE(expected.fmt.extra_data == actual.fmt.extra_data);
    REQUIRE(expected.data.samples.size() == actual.data.samples.size());
    REQUIRE(expected.data.samples == actual.data.samples);

    REQUIRE((expected.smpl == nullptr) == (actual.smpl == nullptr));

    if (expected.smpl)
    {
        REQUIRE(expected.smpl->manufacturer == actual.smpl->manufacturer);
        REQUIRE(expected.smpl->product == actual.smpl->product);
        REQUIRE(expected.smpl->sample_period == actual.smpl->sample_period);
        REQUIRE(expected.smpl->midi_unity_note == actual.smpl->midi_unity_note);
        REQUIRE(expected.smpl->midi_pitch_fraction
            == actual.smpl->midi_pitch_fraction);
        REQUIRE(expected.smpl->smpte_format == actual.smpl->smpte_format);
        REQUIRE(expected.smpl->smpte_offset == actual.smpl->smpte_offset);
        REQUIRE(expected.smpl->extra_data == actual.smpl->extra_data);
        REQUIRE(expected.smpl->loops.size() == actual.smpl->loops.size());
        for (const auto i : util::range(expected.smpl->loops.size()))
        {
            const auto &expected_loop = expected.smpl->loops[i];
            const auto &actual_loop = actual.smpl->loops[i];
            REQUIRE(expected_loop.type == actual_loop.type);
            REQUIRE(expected_loop.start == actual_loop.start);
            REQUIRE(expected_loop.end == actual_loop.end);
            REQUIRE(expected_loop.fraction == actual_loop.fraction);
            REQUIRE(expected_loop.play_count == actual_loop.play_count);
        }
    }
}

void tests::compare_audio(
    io::File &expected_file, const sfx::Wave &actual_audio)
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
