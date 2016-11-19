// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "test_support/audio_support.h"
#include "algo/format.h"
#include "algo/range.h"
#include "dec/microsoft/wav_audio_decoder.h"
#include "test_support/catch.h"
#include "test_support/common.h"
#include "test_support/file_support.h"

using namespace au;

static res::Audio audio_from_file(io::File &file)
{
    Logger dummy_logger;
    dummy_logger.mute();
    const auto wav_audio_decoder = dec::microsoft::WavAudioDecoder();
    return wav_audio_decoder.decode(dummy_logger, file);
}

res::Audio tests::get_test_audio()
{
    io::File input_file(
        "tests/dec/entis/files/mio/explosion-out.wav",
        io::FileMode::Read);
    return audio_from_file(input_file);
}

void tests::compare_audio(
    const res::Audio &actual, const res::Audio &expected)
{
    REQUIRE(actual.codec == expected.codec);
    REQUIRE(actual.channel_count == expected.channel_count);
    REQUIRE(actual.sample_rate == expected.sample_rate);
    REQUIRE(actual.bits_per_sample == expected.bits_per_sample);
    REQUIRE(actual.extra_codec_headers == expected.extra_codec_headers);
    REQUIRE(actual.samples.size() == expected.samples.size());
    tests::compare_binary(actual.samples, expected.samples);

    REQUIRE(actual.loops.size() == expected.loops.size());
    for (const auto i : algo::range(expected.loops.size()))
    {
        const auto &expected_loop = expected.loops[i];
        const auto &actual_loop = actual.loops[i];
        REQUIRE(actual_loop.start == expected_loop.start);
        REQUIRE(actual_loop.end == expected_loop.end);
        REQUIRE(actual_loop.play_count == expected_loop.play_count);
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
    REQUIRE(actual_files.size() == expected_files.size());
    for (const auto i : algo::range(actual_files.size()))
    {
        INFO(algo::format("Audio at index %d differs", i));
        tests::compare_audio(
            *actual_files[i], *expected_files[i], compare_file_paths);
    }
}
