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

#include "enc/microsoft/wav_audio_encoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::enc::microsoft;

void WavAudioEncoder::encode_impl(
    const Logger &logger,
    const res::Audio &input_audio,
    io::File &output_file) const
{
    const auto block_align
        = input_audio.channel_count * input_audio.bits_per_sample / 8;
    const auto byte_rate = input_audio.sample_rate * block_align;

    output_file.stream.write("RIFF"_b);
    output_file.stream.write("\x00\x00\x00\x00"_b);
    output_file.stream.write("WAVE"_b);

    output_file.stream.write("fmt "_b);
    output_file.stream.write_le<u32>(
        18 + input_audio.extra_codec_headers.size());
    output_file.stream.write_le<u16>(input_audio.codec);
    output_file.stream.write_le<u16>(input_audio.channel_count);
    output_file.stream.write_le<u32>(input_audio.sample_rate);
    output_file.stream.write_le<u32>(byte_rate);
    output_file.stream.write_le<u16>(block_align);
    output_file.stream.write_le<u16>(input_audio.bits_per_sample);
    output_file.stream.write_le<u16>(input_audio.extra_codec_headers.size());
    output_file.stream.write(input_audio.extra_codec_headers);

    output_file.stream.write("data"_b);
    output_file.stream.write_le<u32>(input_audio.samples.size());
    output_file.stream.write(input_audio.samples);

    if (!input_audio.loops.empty())
    {
        const auto extra_data = ""_b;
        output_file.stream.write("smpl"_b);
        output_file.stream.write_le<u32>(36
            + (24 * input_audio.loops.size()) + extra_data.size());
        output_file.stream.write_le<u32>(0); // manufacturer
        output_file.stream.write_le<u32>(0); // product
        output_file.stream.write_le<u32>(0); // sample period
        output_file.stream.write_le<u32>(0); // midi unity note
        output_file.stream.write_le<u32>(0); // midi pitch fraction
        output_file.stream.write_le<u32>(0); // smpte format
        output_file.stream.write_le<u32>(0); // smpte offset
        output_file.stream.write_le<u32>(input_audio.loops.size());
        output_file.stream.write_le<u32>(extra_data.size());
        for (const auto i : algo::range(input_audio.loops.size()))
        {
            const auto loop = input_audio.loops[i];
            output_file.stream.write_le<u32>(i);
            output_file.stream.write_le<u32>(0); // type
            output_file.stream.write_le<u32>(loop.start);
            output_file.stream.write_le<u32>(loop.end);
            output_file.stream.write_le<u32>(0); // fraction
            output_file.stream.write_le<u32>(loop.play_count);
        }
        output_file.stream.write(extra_data);
    }

    output_file.stream.seek(4);
    output_file.stream.write_le<u32>(output_file.stream.size() - 8);

    if (!input_audio.loops.empty())
        output_file.path.change_extension("wavloop");
    else
        output_file.path.change_extension("wav");
}
