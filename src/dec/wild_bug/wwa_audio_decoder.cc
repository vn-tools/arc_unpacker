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

#include "dec/wild_bug/wwa_audio_decoder.h"
#include "dec/wild_bug/wpx/decoder.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::wild_bug;

static const bstr magic = "WPX\x1AWAV\x00"_b;

bool WwaAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Audio WwaAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    wpx::Decoder decoder(input_file.stream);
    io::MemoryByteStream metadata_stream(decoder.read_plain_section(0x20));

    const auto pcm_type = metadata_stream.read_le<u16>();
    const auto channel_count = metadata_stream.read_le<u16>();
    const auto sample_rate = metadata_stream.read_le<u32>();
    const auto byte_rate = metadata_stream.read_le<u32>();
    const auto block_align = metadata_stream.read_le<u16>();
    const auto bits_per_sample = metadata_stream.read_le<u16>();

    const auto samples = decoder.read_compressed_section(0x21);

    res::Audio audio;
    audio.codec = pcm_type;
    audio.channel_count = channel_count;
    audio.sample_rate = sample_rate;
    audio.bits_per_sample = bits_per_sample;
    audio.samples = samples;
    return audio;
}

static auto _ = dec::register_decoder<WwaAudioDecoder>("wild-bug/wwa");
