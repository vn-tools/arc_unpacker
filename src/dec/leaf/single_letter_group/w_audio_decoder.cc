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

#include "dec/leaf/single_letter_group/w_audio_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "bw\x20\x20"_b;

bool WAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("w"))
        return false;
    const auto data_size = input_file.stream.seek(10).read_le<u32>();
    return data_size + 18 == input_file.stream.size();
}

res::Audio WAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    res::Audio audio;
    audio.channel_count = input_file.stream.read<u8>();
    const auto block_align = input_file.stream.read<u8>();
    audio.sample_rate = input_file.stream.read_le<u16>();
    audio.bits_per_sample = input_file.stream.read_le<u16>();
    const auto byte_rate = input_file.stream.read_le<u32>();
    const auto samples_size = input_file.stream.read_le<u32>();
    const auto loop_pos = input_file.stream.read_le<u32>();
    if (loop_pos)
        audio.loops.push_back(res::AudioLoopInfo {loop_pos, samples_size, 0});
    audio.samples = input_file.stream.read(samples_size);
    return audio;
}

static auto _ = dec::register_decoder<WAudioDecoder>("leaf/w");
