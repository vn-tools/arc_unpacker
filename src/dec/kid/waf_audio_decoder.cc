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

#include "dec/kid/waf_audio_decoder.h"

using namespace au;
using namespace au::dec::kid;

static const bstr magic = "WAF\x00\x00\x00"_b;

bool WafAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Audio WafAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    auto output_file = std::make_unique<io::File>();

    input_file.stream.seek(6);

    res::Audio audio;
    audio.codec = 2;
    audio.channel_count = input_file.stream.read_le<u16>();
    audio.sample_rate = input_file.stream.read_le<u32>();
    const auto byte_rate = input_file.stream.read_le<u32>();
    const auto block_align = input_file.stream.read_le<u16>();
    audio.bits_per_sample = input_file.stream.read_le<u16>();
    audio.extra_codec_headers = input_file.stream.read(32);
    const auto samples_size = input_file.stream.read_le<u32>();
    audio.samples = input_file.stream.read(samples_size);
    return audio;
}

static auto _ = dec::register_decoder<WafAudioDecoder>("kid/waf");
