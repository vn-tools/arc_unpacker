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

#include "dec/microsoft/wav_audio_decoder.h"
#include "algo/str.h"
#include "err.h"

using namespace au;
using namespace au::dec::microsoft;

static const bstr riff_magic = "RIFF"_b;
static const bstr wave_magic = "WAVE"_b;

bool WavAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(riff_magic.size()) == riff_magic
        && input_file.stream.seek(8).read(wave_magic.size()) == wave_magic;
}

res::Audio WavAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    res::Audio audio;
    input_file.stream.seek(12);
    bool head_found = false;
    bool data_found = false;

    while (input_file.stream.left() >= 8)
    {
        const auto chunk_name = input_file.stream.read(4).str(true);
        const auto chunk_size = input_file.stream.read_le<u32>();
        const auto chunk_start = input_file.stream.pos();

        if (chunk_name == "fmt\x20")
        {
            audio.codec = input_file.stream.read_le<u16>();
            audio.channel_count = input_file.stream.read_le<u16>();
            audio.sample_rate = input_file.stream.read_le<u32>();
            const auto byte_rate = input_file.stream.read_le<u32>();
            const auto block_align = input_file.stream.read_le<u16>();
            audio.bits_per_sample = input_file.stream.read_le<u16>();
            const auto chunk_pos = input_file.stream.pos() - chunk_start;
            if (chunk_pos < chunk_size)
            {
                const auto extra_data_size = input_file.stream.read_le<u16>();
                audio.extra_codec_headers
                    = input_file.stream.read(extra_data_size);
            }
            input_file.stream.seek(chunk_start + chunk_size);
            head_found = true;
        }
        else if (chunk_name == "data")
        {
            audio.samples = input_file.stream.read(chunk_size);
            data_found = true;
        }
        else
        {
            logger.warn("Unknown chunk: %s\n", algo::hex(chunk_name).c_str());
            if (input_file.stream.left() < chunk_size)
            {
                logger.warn("Invalid chunk size, ignoring rest of the file\n");
                break;
            }
            else
            {
                input_file.stream.skip(chunk_size);
            }
        }
    }

    if (!head_found)
        throw err::CorruptDataError("No header data found");
    if (!data_found)
        throw err::CorruptDataError("No samples data found");

    return audio;
}

static auto _ = dec::register_decoder<WavAudioDecoder>("microsoft/wav");
