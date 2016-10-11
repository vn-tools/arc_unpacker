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

#include "dec/twilight_frontier/pak2_audio_decoder.h"

using namespace au;
using namespace au::dec::twilight_frontier;

bool Pak2AudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("cv3");
}

res::Audio Pak2AudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto format = input_file.stream.read_le<u16>();
    const auto channel_count = input_file.stream.read_le<u16>();
    const auto sample_rate = input_file.stream.read_le<u32>();
    const auto byte_rate = input_file.stream.read_le<u32>();
    const auto block_align = input_file.stream.read_le<u16>();
    const auto bits_per_sample = input_file.stream.read_le<u16>();
    input_file.stream.skip(2);
    const auto size = input_file.stream.read_le<u32>();
    const auto samples = input_file.stream.read(size);

    res::Audio audio;
    audio.channel_count = channel_count;
    audio.bits_per_sample = bits_per_sample;
    audio.sample_rate = sample_rate;
    audio.samples = samples;
    return audio;
}

static auto _ = dec::register_decoder<Pak2AudioDecoder>(
    "twilight-frontier/pak2-sfx");
